#include "Rendering/AnimationSystem.h"

AnimationSystem::AnimationSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EAutoAnimationBlend, &AnimationSystem::OnAutoAnimationBlend);
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

        Skeleton* skeleton = model->m_RawModel->m_Skeleton;
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
            return;
        }

        Skeleton* skeleton = model->m_RawModel->m_Skeleton;
        if (skeleton == nullptr) {
            return;
        }

        const Skeleton::Animation* animation = skeleton->GetAnimation(animationC["AnimationName"]);

        if (animation == nullptr) {
            continue;;
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
    for (auto& autoBlendQueue : m_AutoBlendQueues) {

        if(autoBlendQueue.second.HasActiveBlendJob()) {
            AutoBlendQueue::AutoBlendJob& blendJob = autoBlendQueue.second.GetActiveBlendJob();

            std::shared_ptr<BlendTree> blendTree = autoBlendQueue.second.GetBlendTree();

            if (blendTree != nullptr) {
                if (blendJob.Duration != 0.0) {
                    blendJob.BlendInfo.progress = glm::clamp(blendJob.CurrentTime / blendJob.Duration, 0.0, 1.0);
                } else {
                    blendJob.BlendInfo.progress = 1.0;
                }

                blendJob.BlendInfo = blendTree->AutoBlendStep(blendJob.BlendInfo);
            }

        }

    }
}

bool AnimationSystem::OnAutoAnimationBlend(Events::AutoAnimationBlend& e)
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

    Skeleton* skeleton = model->m_RawModel->m_Skeleton;
    if (skeleton == nullptr) {
        return false;
    }

    std::shared_ptr<BlendTree> blendTree;
    if (skeleton->BlendTrees.find(e.RootNode) != skeleton->BlendTrees.end()) {
        blendTree = skeleton->BlendTrees.at(e.RootNode);
    } else {
        return false;
    }



    EntityWrapper subTreeRoot = blendTree->GetSubTreeRoot(e.NodeName);

    if (!subTreeRoot.Valid()) {
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

    EntityWrapper nodeEntity = subTreeRoot.FirstChildByName(e.NodeName); // more than one
    if (nodeEntity.Valid()) {
        if (nodeEntity.HasComponent("Animation")) {
            const Skeleton::Animation* animation = skeleton->GetAnimation(nodeEntity["Animation"]["AnimationName"]);
            (Field<bool>)nodeEntity["Animation"]["Reverse"] = e.Reverse;

            if (e.Restart) {
                if (animation != nullptr) {
                    if (e.Restart) {
                        if (e.Reverse) {
                            (Field<double>)nodeEntity["Animation"]["Time"] = animation->Duration;
                        } else {
                            (Field<double>)nodeEntity["Animation"]["Time"] = 0.0;
                        }
                    }
                }
            }
        }
    }
    m_AutoBlendQueues[subTreeRoot].Insert(abj);
    return true;
}
