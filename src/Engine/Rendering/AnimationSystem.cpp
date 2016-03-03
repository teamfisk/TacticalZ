#include "Rendering/AnimationSystem.h"

AnimationSystem::AnimationSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EAnimationBlend, &AnimationSystem::OnAnimationBlend);
    EVENT_SUBSCRIBE_MEMBER(m_EAutoAnimationBlend, &AnimationSystem::OnAutoAnimationBlend);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &AnimationSystem::OnInputCommand);
}

void AnimationSystem::Update(double dt)
{
    ImGui::InputText("AnimationName1", &m_AnimationName1[0], sizeof(m_AnimationName1));
    ImGui::SliderFloat("Blendtime1", &m_BlendTime1, 0.f, 10.f);
    ImGui::InputText("AnimationName2", &m_AnimationName2[0], sizeof(m_AnimationName2));
    ImGui::SliderFloat("Blendtime2", &m_BlendTime2, 0.f, 10.f);
    
    UpdateAnimations(dt);
    CreateBlendTrees();
    UpdateWeights(dt);
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

        if (animationSpeed != 0.0) {

            double nextTime = (double)animationC["Time"] + animationSpeed * dt;


            //Pre animation end blend
            if (m_QueuedAutoBlendJobs.find(entity) != m_QueuedAutoBlendJobs.end()) {
                if (glm::sign(m_QueuedAutoBlendJobs.at(entity).Delay) < 0) {
                    if (!(bool)animationC["Loop"]) {
                        if (nextTime > animation->Duration + m_QueuedAutoBlendJobs.at(entity).Delay  && glm::sign(animationSpeed) > 0) {
                            AnimationComplete(entity);
                        } else if (nextTime < 0 - m_QueuedAutoBlendJobs.at(entity).Delay && glm::sign(animationSpeed) < 0) {
                            AnimationComplete(entity);
                        }
                    } else {
                        if (nextTime > animation->Duration + m_QueuedAutoBlendJobs.at(entity).Delay && glm::sign(animationSpeed) > 0) {
                            AnimationComplete(entity);
                        } else if (nextTime < 0 - m_QueuedAutoBlendJobs.at(entity).Delay && glm::sign(animationSpeed) < 0) {
                            AnimationComplete(entity);
                        }
                    }
                }
            }


            if (!(bool)animationC["Loop"]) {
                if (nextTime > animation->Duration) {
                    nextTime = animation->Duration;
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationC["AnimationName"];
                    m_EventBroker->Publish(e);
                    AnimationComplete(entity);
                    (double&)animationC["Speed"] = 0.0;
                } else if (nextTime < 0) {
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationC["AnimationName"];
                    m_EventBroker->Publish(e);
                    AnimationComplete(entity);
                    nextTime = 0;
                    (double&)animationC["Speed"] = 0.0;
                }
            } else {
                if (nextTime > animation->Duration) {
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationC["AnimationName"];
                    m_EventBroker->Publish(e);
                    AnimationComplete(entity);

                    while (nextTime > animation->Duration) {
                        nextTime -= animation->Duration;
                    }
                } else if (nextTime < 0) {
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationC["AnimationName"];
                    m_EventBroker->Publish(e);
                    AnimationComplete(entity);
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
    for (auto it = m_BlendJobs.begin(); it != m_BlendJobs.end();) {
        if (!it->BlendEntity.Valid()) {
            it = m_BlendJobs.erase(it);
            continue;
        }

        if (it->BlendEntity.HasComponent("Blend")) {
            it->CurrentTime += dt;
            double progress = it->CurrentTime / it->Duration;
            progress = glm::clamp(progress, 0.0, 1.0);

            double weight = ((it->GoalWeight - it->StartWeight) * progress) + it->StartWeight;
            (double&)it->BlendEntity["Blend"]["Weight"] = weight;

            if(weight == it->GoalWeight) {
                it = m_BlendJobs.erase(it);
            }
        }

        ++it;
    }


    for (auto it = m_AutoBlendJobs.begin(); it != m_AutoBlendJobs.end();) {
        it->CurrentTime += dt;

        if (!it->RootNode.Valid()) {
            it = m_AutoBlendJobs.erase(it);
            continue;
        }

        if (!it->RootNode.HasComponent("Model")) {
            it = m_AutoBlendJobs.erase(it);
            continue;
        }

        Model* model;
        try {
            model = ResourceManager::Load<::Model, true>(it->RootNode["Model"]["Resource"]);
        } catch (const std::exception&) {
            continue;
        }

        Skeleton* skeleton = model->m_RawModel->m_Skeleton;
        if (skeleton == nullptr) {
            continue;
        }

        std::shared_ptr<BlendTree> blendTree;
        if(skeleton->BlendTrees.find(it->RootNode) != skeleton->BlendTrees.end()) {
            blendTree = skeleton->BlendTrees.at(it->RootNode);
        } else {
            it = m_AutoBlendJobs.erase(it);
            continue;
        }
        

        it->BlendInfo.progress = glm::clamp(it->CurrentTime / it->Duration, 0.0, 1.0);
        it->BlendInfo = blendTree->AutoBlendStep(it->BlendInfo);


        if (it->CurrentTime >= it->Duration) {
            it = m_AutoBlendJobs.erase(it);
            continue;
        }

        ++it;
    }
}


void AnimationSystem::AnimationComplete(EntityWrapper animationEntity)
{
    for (auto it = m_QueuedBlendJobs.begin(); it != m_QueuedBlendJobs.end();) {
        if (!it->BlendEntity.Valid() || !it->AnimationEntity.Valid()) {
            it = m_QueuedBlendJobs.erase(it);
            continue;
        }

        if(animationEntity == it->AnimationEntity) {
            BlendJob bj;
            bj.BlendEntity = it->BlendEntity;
            bj.StartWeight = it->StartWeight;
            bj.GoalWeight = it->GoalWeight;
            bj.Duration = it->Duration;
            bj.CurrentTime = 0.0;
            m_BlendJobs.push_back(bj);
            it = m_QueuedBlendJobs.erase(it);
            continue;
        }

        ++it;
    }


    if (m_QueuedAutoBlendJobs.find(animationEntity) != m_QueuedAutoBlendJobs.end()) {
        AutoBlendJob abj = m_QueuedAutoBlendJobs.at(animationEntity);

        if (!abj.RootNode.Valid() || !animationEntity.Valid()) {
            m_QueuedAutoBlendJobs.erase(animationEntity);
        } else {
            m_AutoBlendJobs.push_back(abj);
            m_QueuedAutoBlendJobs.erase(animationEntity);
        }



    }


}

bool AnimationSystem::OnAnimationBlend(Events::AnimationBlend& e)
{
    if(!e.BlendEntity.Valid()) {
        return false;
    }
     if(!e.BlendEntity.HasComponent("Blend")){
         return false;
     }

     if (e.AnimationEntity.Valid()) {
         if (e.AnimationEntity.HasComponent("Animation")) {
             QueuedBlendJob qbj;
             qbj.BlendEntity = e.BlendEntity;
             qbj.StartWeight = (double)e.BlendEntity["Blend"]["Weight"];
             qbj.GoalWeight = e.GoalWeight;
             qbj.Duration = e.Duration;
             qbj.CurrentTime = 0.0;
             qbj.AnimationEntity = e.AnimationEntity;
             m_QueuedBlendJobs.push_back(qbj);
             return true;
         }
     } else {
         BlendJob bj;
         bj.BlendEntity = e.BlendEntity;
         bj.StartWeight = (double)e.BlendEntity["Blend"]["Weight"];
         bj.GoalWeight = e.GoalWeight;
         bj.Duration = e.Duration;
         bj.CurrentTime = 0.0;
         m_BlendJobs.push_back(bj);
         return true;
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

    if (e.AnimationEntity.Valid()) {
        AutoBlendJob abj;
        abj.RootNode = e.RootNode;
        abj.CurrentTime = 0.0;
        abj.Duration = e.Duration;
        abj.Delay = e.Delay;

        BlendTree::AutoBlendInfo abInfo;
        abInfo.NodeName = e.NodeName;
        abInfo.progress = 0.0;
        abInfo.Restart = e.Restart;
        abInfo.AnimationSpeed = e.AnimationSpeed;
        abj.BlendInfo = abInfo;

        m_QueuedAutoBlendJobs[e.AnimationEntity] = abj;
        return true;
    } else {
        AutoBlendJob abj;
        abj.RootNode = e.RootNode;
        abj.CurrentTime = 0.0;
        abj.Duration = e.Duration;

        BlendTree::AutoBlendInfo abInfo;
        abInfo.NodeName = e.NodeName;
        abInfo.progress = 0.0;
        abInfo.Restart = e.Restart;
        abInfo.AnimationSpeed = e.AnimationSpeed;

        abj.BlendInfo = abInfo;

        m_AutoBlendJobs.push_back(abj);
        return true;
    }


}

bool AnimationSystem::OnInputCommand(const Events::InputCommand& e)
{

    if (e.Value == 1.f) {
        if (e.Command == "BlendTest0") {
            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }


            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {
                    Events::AutoAnimationBlend aeb;
                    aeb.Duration = m_BlendTime1;
                    aeb.NodeName = m_AnimationName1;
                    aeb.RootNode = entity;
                    aeb.Restart = true;

                    m_EventBroker->Publish(aeb);

                }
            }

        } else  if (e.Command == "BlendTest1") {

            auto blendComponents = m_World->GetComponents("BlendAdditive");

            if (blendComponents == nullptr) {
                return false;
            }


            for (auto& bc : *blendComponents) {
                EntityWrapper entity = EntityWrapper(m_World, bc.EntityID);

                if (entity.Name() == "Assault") {

                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = m_BlendTime2;
                        aeb.NodeName = m_AnimationName2;
                        aeb.RootNode = entity;
                        aeb.Restart = true;
                        m_EventBroker->Publish(aeb);
                    }

                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = m_BlendTime2;
                        aeb.NodeName = "Run";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
                        aeb.AnimationEntity = entity.FirstChildByName("DashLeft");
                        m_EventBroker->Publish(aeb);
                    }

                }
            }
        }


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
                        aeb.Duration = 0.1;
                        aeb.NodeName = "DashForward";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "Run";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
                        aeb.Delay = 0;
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
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "Run";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
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
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "Run";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
                        aeb.Delay = -0.3;
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
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.3;
                        aeb.NodeName = "Run";
                        aeb.RootNode = entity;
                        aeb.Restart = true;
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
                        aeb.Restart = true;
                        m_EventBroker->Publish(aeb);
                    }
                    {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.35;
                        aeb.NodeName = "Run";
                        aeb.RootNode = entity;
                        aeb.Restart = false;
                        aeb.AnimationEntity = entity.FirstChildByName("Jump");
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        }


    }
}

