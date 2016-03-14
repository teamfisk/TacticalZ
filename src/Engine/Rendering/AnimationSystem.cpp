#include "Rendering/AnimationSystem.h"

AnimationSystem::AnimationSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EAutoAnimationBlend, &AnimationSystem::OnAutoAnimationBlend);
    EVENT_SUBSCRIBE_MEMBER(m_EEntityDeleted, &AnimationSystem::OnEntityDeleted);
    EVENT_SUBSCRIBE_MEMBER(m_ESetBlendWeight, &AnimationSystem::OnSetBlendWeight);
}

void AnimationSystem::Update(double dt)
{
    UpdateAnimations(dt);
    CreateBlendTrees();
    UpdateWeights(dt);


    for(auto& autoBlendQueue : m_AutoBlendQueues) {
        autoBlendQueue.second.UpdateTime(dt);
    }
}

void AnimationSystem::CreateBlendTrees()
{
    auto modelComponents = m_World->GetComponents("Model");
    if (modelComponents == nullptr) {
        return;
    }

    for (auto& modelC : *modelComponents) {
        EntityWrapper entity = EntityWrapper(m_World, modelC.EntityID);

        Model* model;
        try {
            model = ResourceManager::Load<::Model, true>(entity["Model"]["Resource"]);
        } catch (const std::exception&) {
            continue;;
        }

        Skeleton* skeleton = model->m_Skeleton;
        if (skeleton == nullptr) {
            continue;
        }

        if (entity.HasComponent("Blend") || entity.HasComponent("BlendOverride") ||
            entity.HasComponent("BlendAdditive") || entity.HasComponent("Animation"))
        {
            std::shared_ptr<BlendTree> blendTree = std::shared_ptr<BlendTree>(new BlendTree(entity, skeleton));

            if(blendTree->IsValid()) {
                skeleton->BlendTrees[entity] = blendTree;
            }
            
        }
    }
}

void AnimationSystem::UpdateAnimations(double dt)
{
    auto animationComponents = m_World->GetComponents("Animation");
    if(animationComponents == nullptr) {
        return;
    }

    for (auto& animationC : *animationComponents) {
        EntityWrapper entity = EntityWrapper(m_World, animationC.EntityID);
        EntityWrapper modelEntity;
        if(!entity.HasComponent("Model")) {
            modelEntity = entity.FirstParentWithComponent("Model");
        } else {
            modelEntity = entity;
        }

        Model* model;
        try {
            Field<std::string> res = modelEntity["Model"]["Resource"];
            model = ResourceManager::Load<::Model, true>(res);
        } catch (const std::exception&) {
            continue;
        }

        Skeleton* skeleton = model->m_Skeleton;
        if (skeleton == nullptr) {
            continue;
        }

        const Skeleton::Animation* animation = skeleton->GetAnimation(animationC["AnimationName"]);

        if (animation == nullptr) {
            continue;
        }

        double animationSpeed = (const double&)animationC["Speed"];

        if((const bool&)animationC["Reverse"]) {
            animationSpeed *= -1;
        }


        if ((const bool&)animationC["Play"]) {

            double nextTime = (Field<double>)animationC["Time"] + animationSpeed * dt;
            if (!(Field<bool>)animationC["Loop"]) {
                if (nextTime > animation->Duration) {
                    nextTime = animation->Duration;
                    (Field<bool>)animationC["Play"] = false;
                } else if (nextTime < 0) {
                    nextTime = 0;
                    (Field<bool>)animationC["Play"] = false;
                }
            } else {
                if (nextTime > animation->Duration) {

                    while (nextTime > animation->Duration) {
                        nextTime -= animation->Duration;
                    }
                } else if (nextTime < 0) {
                    while (nextTime < 0) {
                        nextTime += animation->Duration;
                    }
                }
            }
            (Field<double>)animationC["Time"] = nextTime;
        }
    }
}

void AnimationSystem::UpdateWeights(double dt)
{
    for (auto it = m_AutoBlendQueues.begin(); it != m_AutoBlendQueues.end(); ) {
        //LOG_INFO("%s", it->first.Name().c_str());
        //it->second.PrintQueue();
        
        if(it->second.HasActiveBlendJob()) {
            AutoBlendQueue::AutoBlendJob& blendJob = it->second.GetActiveBlendJob();
            //LOG_INFO("%s", blendJob.RootNode.Name().c_str());
            std::shared_ptr<BlendTree> blendTree = it->second.GetBlendTree();
            if (blendTree != nullptr) {
                if (blendJob.Duration != 0.0) {
                    blendJob.BlendInfo.progress = glm::clamp(blendJob.CurrentTime / blendJob.Duration, 0.0, 1.0);
                } else {
                    blendJob.BlendInfo.progress = 1.0;
                }

                blendJob.BlendInfo = blendTree->AutoBlendStep(blendJob.BlendInfo);
            } else {
                it = m_AutoBlendQueues.erase(it);
            }
            it++;
        } else {
            if (it->second.Empty()) {
                it = m_AutoBlendQueues.erase(it);
            } else {
                it++;
            }
        }
    }
}

bool AnimationSystem::OnAutoAnimationBlend(Events::AutoAnimationBlend& e)
{
    if (!e.RootNode.Valid()) {
        LOG_ERROR("%s, RootNode invalid %s", e.NodeName, e.RootNode.Name().c_str());
        return false;
    }

    if (!e.RootNode.HasComponent("Model")) {
        LOG_ERROR("%s, RootNode has no model %s", e.NodeName, e.RootNode.Name().c_str());
        return false;
    }

    Model* model;
    try {
        model = ResourceManager::Load<::Model, true>((std::string)e.RootNode["Model"]["Resource"]);
    } catch (const std::exception&) {
        LOG_ERROR("%s, RootNode model not finished loading %s", e.NodeName, e.RootNode.Name().c_str());
        return false;
    }

    Skeleton* skeleton = model->m_Skeleton;
    if (skeleton == nullptr) {
        LOG_ERROR("%s, RootNode skeleton invalid %s", e.NodeName, e.RootNode.Name().c_str());
        return false;
    }

    std::shared_ptr<BlendTree> blendTree;
    if (skeleton->BlendTrees.find(e.RootNode) != skeleton->BlendTrees.end()) {
        blendTree = skeleton->BlendTrees.at(e.RootNode);
    } else {
        LOG_ERROR("%s, No blendtree was found invalid %s", e.NodeName, e.RootNode.Name().c_str());
        return false;
    }

    EntityWrapper subTreeRoot;


    if (e.SingleLevelBlend) {
        subTreeRoot = blendTree->GetSubTreeRoot(e.NodeName);

        if (!subTreeRoot.Valid()) {
            LOG_ERROR("%s, subTreeRoot invalid", e.NodeName);
            return false;
        }

        AutoBlendQueue::AutoBlendJob abj;
        abj.AnimationEntity = e.AnimationEntity;
        abj.CurrentTime = 0.0;
        abj.Delay = e.Delay;
        abj.Duration = e.Duration;
        abj.RootNode = e.RootNode;

        abj.BlendInfo.NodeName = e.NodeName;
        abj.BlendInfo.progress = 0.0;
        abj.BlendInfo.Start = e.Start;
        abj.BlendInfo.SingleBlend = e.SingleLevelBlend;
        abj.BlendInfo.Weight = e.Weight;

        std::vector<EntityWrapper> animationEntities = blendTree->GetEntitesByName(e.NodeName); // more than one
        for(auto entity : animationEntities)
        if (entity.Valid()) {
            if (entity.HasComponent("Animation")) {
                const Skeleton::Animation* animation = skeleton->GetAnimation(entity["Animation"]["AnimationName"]);
                (Field<bool>)entity["Animation"]["Reverse"] = e.Reverse;

                if (e.Restart) {
                    if (animation != nullptr) {
                        if (e.Restart) {
                            if (e.Reverse) {
                                (Field<double>)entity["Animation"]["Time"] = animation->Duration;
                            } else {
                                (Field<double>)entity["Animation"]["Time"] = 0.0;
                            }
                        }
                    }
                }
            }
        }
        m_AutoBlendQueues[subTreeRoot].Insert(abj);
    } else {
        std::vector<EntityWrapper> subtreeroots = blendTree->GetSingleLevelRoots(e.NodeName);

        for(auto entity : subtreeroots) {
            subTreeRoot = entity;

            if (!subTreeRoot.Valid()) {
                LOG_ERROR("%s, subTreeRoot invalid", e.NodeName);
                return false;
            }

            AutoBlendQueue::AutoBlendJob abj;
            abj.AnimationEntity = e.AnimationEntity;
            abj.CurrentTime = 0.0;
            abj.Delay = e.Delay;
            abj.Duration = e.Duration;
            abj.RootNode = e.RootNode;

            abj.BlendInfo.NodeName = e.NodeName;
            abj.BlendInfo.progress = 0.0;
            abj.BlendInfo.Start = e.Start;
            abj.BlendInfo.SingleBlend = e.SingleLevelBlend;
            abj.BlendInfo.Weight = e.Weight;
            m_AutoBlendQueues[subTreeRoot].Insert(abj);
        }

        std::vector<EntityWrapper> animationEntities = blendTree->GetEntitesByName(e.NodeName); // more than one
        for (auto entity : animationEntities)
            if (entity.Valid()) {
                if (entity.HasComponent("Animation")) {
                    const Skeleton::Animation* animation = skeleton->GetAnimation(entity["Animation"]["AnimationName"]);
                    entity["Animation"]["Reverse"] = e.Reverse;

                    if (e.Restart) {
                        if (animation != nullptr) {
                            if (e.Restart) {
                                if (e.Reverse) {
                                    entity["Animation"]["Time"] = animation->Duration;
                                } else {
                                    entity["Animation"]["Time"] = 0.0;
                                }
                            }
                        }
                    }
                }
            }
    }

   
    return true;
}

bool AnimationSystem::OnEntityDeleted(Events::EntityDeleted& e)
{
    EntityWrapper entity = e.DeletedEntity;

    if (entity.HasComponent("Model")) {
        Model* model;
        try {
            model = ResourceManager::Load<::Model, true>(entity["Model"]["Resource"]);
        } catch (const std::exception&) {
            return false;
        }

        Skeleton* skeleton = model->m_Skeleton;
        if (skeleton == nullptr) {
            return false;
        }

        if (skeleton->BlendTrees.find(entity) != skeleton->BlendTrees.end()) {
            skeleton->BlendTrees.erase(entity);
        }


    }

}

bool AnimationSystem::OnSetBlendWeight(Events::SetBlendWeight& e)
{
    if (!e.RootNode.Valid()) {
        return false;
    }

    if (!e.RootNode.HasComponent("Model")) {
        return false;
    }

    Model* model;
    try {
        model = ResourceManager::Load<::Model, true>((std::string)e.RootNode["Model"]["Resource"]);
    } catch (const std::exception&) {
        return false;
    }

    Skeleton* skeleton = model->m_Skeleton;
    if (skeleton == nullptr) {
        return false;
    }

    std::shared_ptr<BlendTree> blendTree;
    if (skeleton->BlendTrees.find(e.RootNode) != skeleton->BlendTrees.end()) {
        blendTree = skeleton->BlendTrees.at(e.RootNode);
    } else {
        return false;
    }

    if(e.Weight >= 0 && e.Weight <= 1) {

        blendTree->SetWeightByName(e.NodeName, e.Weight);

        return true;
    } else {
        return false;
    }

}
