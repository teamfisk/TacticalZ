#include "Rendering/AnimationSystem.h"

AnimationSystem::AnimationSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EAutoAnimationBlend, &AnimationSystem::OnAutoAnimationBlend);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &AnimationSystem::OnInputCommand);
}

void AnimationSystem::Update(double dt)
{
    UpdateAnimations(dt);
    CreateBlendTrees();
    UpdateWeights(dt);


    for(auto& autoBlendQueue : m_AutoBlendQueues) {
        autoBlendQueue.second.UpdateTime(dt);
     //   autoBlendQueue.second.PrintQueue();
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
            model = ResourceManager::Load<::Model, true>(modelEntity["Model"]["Resource"]);
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

        double animationSpeed = (double)animationC["Speed"];

        if((bool)animationC["Reverse"]) {
            animationSpeed *= -1;
        }


        if ((bool)animationC["Play"]) {

            double nextTime = (double)animationC["Time"] + animationSpeed * dt;
            if (!(bool)animationC["Loop"]) {
                if (nextTime > animation->Duration) {
                    nextTime = animation->Duration;
                    (bool&)animationC["Play"] = false;
                } else if (nextTime < 0) {
                    nextTime = 0;
                    (bool&)animationC["Play"] = false;
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
            (double&)animationC["Time"] = nextTime;
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
                blendJob.BlendInfo.progress = glm::clamp(blendJob.CurrentTime / blendJob.Duration, 0.0, 1.0);
                LOG_INFO("Progress: %f, %s", blendJob.BlendInfo.progress, blendJob.BlendInfo.NodeName.c_str());
                blendJob.BlendInfo = blendTree->AutoBlendStep(blendJob.BlendInfo);
            }

        }

    }
}

bool AnimationSystem::OnAutoAnimationBlend(Events::AutoAnimationBlend& e)
{

    if(!e.RootNode.Valid()) {
        return false;
    }

    if(!e.RootNode.HasComponent("Model")) {
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

    if(!subTreeRoot.Valid()) {
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

    if (e.Restart) {
        EntityWrapper nodeEntity = subTreeRoot.FirstChildByName(e.NodeName);
        if (nodeEntity.Valid()) {
            if (nodeEntity.HasComponent("Animation")) {
                const Skeleton::Animation* animation = skeleton->GetAnimation(nodeEntity["Animation"]["AnimationName"]);

                if (animation != nullptr) {
                    if (e.Restart) {
                        (bool&)nodeEntity["Animation"]["Reverse"] = e.Reverse;
                        if (e.Reverse) {
                            (double&)nodeEntity["Animation"]["Time"] = animation->Duration;
                        } else {
                            (double&)nodeEntity["Animation"]["Time"] = 0.0;
                        }
                    }
                }
            }
        }
    }


    LOG_INFO("Inserting %s blendJob into %s subtree", e.NodeName.c_str(), subTreeRoot.Name().c_str());
    m_AutoBlendQueues[subTreeRoot].Insert(abj);

    LOG_INFO("\n");
    m_AutoBlendQueues.at(subTreeRoot).PrintQueue();
    LOG_INFO("\n");


    return true;
}

bool AnimationSystem::OnInputCommand(const Events::InputCommand& e)
{

    if (e.Value == 1.f) {
        if(e.Command == "DashForward") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }
            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.2;
                        aeb.NodeName = "DashForward";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
                        aeb.Start = true;
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "StandCrouchBlend";
                        aeb.RootNode = entity;
                        aeb.Delay = -0.3;
                        aeb.Start = true;
                        aeb.Restart = false;
                        aeb.AnimationEntity = entity.FirstChildByName("DashForward");
                        m_EventBroker->Publish(aeb);
                    }
                }
            }

        } else if (e.Command == "DashBackward") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }
            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.1;
                        aeb.NodeName = "DashBackward";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
                        aeb.Start = true;
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "StandCrouchBlend";
                        aeb.RootNode = entity;
                        aeb.Start = true;
                        aeb.Restart = false;
                        aeb.Delay = -0.3;
                        aeb.AnimationEntity = entity.FirstChildByName("DashBackward");
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        } else if (e.Command == "DashLeft") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }
            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.1;
                        aeb.NodeName = "DashLeft";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
                        aeb.Start = true;
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "StandCrouchBlend";
                        aeb.RootNode = entity;
                        aeb.Delay = -0.3;
                        aeb.Start = true;
                        aeb.Restart = false;
                        aeb.AnimationEntity = entity.FirstChildByName("DashLeft");
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        } else if (e.Command == "DashRight") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }
            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.1;
                        aeb.NodeName = "DashRight";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
                        aeb.Start = true;
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "StandCrouchBlend";
                        aeb.RootNode = entity;
                        aeb.Start = true;
                        aeb.Restart = false;
                        aeb.AnimationEntity = entity.FirstChildByName("DashRight");
                        aeb.Delay = -0.3;
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        } else if (e.Command == "Jump") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }
            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.35;
                        aeb.NodeName = "Jump";
                        aeb.RootNode = entity;
                        aeb.Start = true;
                        aeb.Restart = true;
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.6;
                        aeb.NodeName = "StandCrouchBlend";
                        aeb.RootNode = entity;
                        aeb.Start = true;
                        aeb.Restart = false;
                        aeb.AnimationEntity = entity.FirstChildByName("Jump");
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        } else if (e.Command == "Reload") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }
            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.2;
                        aeb.NodeName = "ReloadSwitch";
                        aeb.RootNode = entity;
                        aeb.Start = true;
                        aeb.Restart = true;
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.2;
                        aeb.NodeName = "IdlePrimary";
                        aeb.RootNode = entity;
                        aeb.Delay = -0.1;
                        aeb.Start = true;
                        aeb.Restart = false;
                        aeb.AnimationEntity = entity.FirstChildByName("ReloadSwitch");
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        } else if (e.Command == "Crouch") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }
            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "CrouchMovement";
                        aeb.RootNode = entity;
                        aeb.Start = true;
                        aeb.Restart = true;
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        } else if (e.Command == "Stand") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }
            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "StandMovement";
                        aeb.RootNode = entity;
                        aeb.Start = true;
                        aeb.Restart = true;
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        } else if (e.Command == "Shoot") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }
            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.1;
                        aeb.NodeName = "ShootPrimary";
                        aeb.RootNode = entity;
                        aeb.Start = true;
                        aeb.Restart = true;
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.1;
                        aeb.NodeName = "IdlePrimary";
                        aeb.RootNode = entity;
                        aeb.Delay = 0.0;
                        aeb.Start = true;
                        aeb.Restart = false;
                        aeb.AnimationEntity = entity.FirstChildByName("ShootPrimary");
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        }


    }
}

