#include "Rendering/AnimationSystem.h"

AnimationSystem::AnimationSystem(SystemParams params)
    : System(params)
{

}

void AnimationSystem::Update(double dt)
{
    UpdateAnimations(dt);
    CreateBlendTrees();
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

        skeleton->BlendTrees[entity] = std::shared_ptr<BlendTree>(new BlendTree(entity, skeleton));
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
        EntityWrapper parent = entity.FirstParentWithComponent("Model");

        Model* model;
        try {
            model = ResourceManager::Load<::Model, true>(parent["Model"]["Resource"]);
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


            if (!(bool)animationC["Loop"]) {
                if (nextTime > animation->Duration) {
                    nextTime = animation->Duration;
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationC["AnimationName"];
                    m_EventBroker->Publish(e);
                } else if (nextTime < 0) {
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationC["AnimationName"];
                    m_EventBroker->Publish(e);
                    nextTime = 0;
                }

                (double&)animationC["Speed"] = 0.0;

            } else {
                if (nextTime > animation->Duration) {
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationC["AnimationName"];
                    m_EventBroker->Publish(e);

                    while (nextTime > animation->Duration) {
                        nextTime -= animation->Duration;
                    }
                } else if (nextTime < 0) {
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationC["AnimationName"];
                    m_EventBroker->Publish(e);

                    while (nextTime < 0) {
                        nextTime += animation->Duration;
                    }
                }
            }

            (double&)animationC["Time"] = nextTime;
        }
    }
}

