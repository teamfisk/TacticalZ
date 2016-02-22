#include "Rendering/AnimationSystem.h"

void AnimationSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& animationComponent, double dt)
{
   
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

    for (int i = 1; i <= 1; i++) {
        const Skeleton::Animation* animation = skeleton->GetAnimation(animationComponent["AnimationName"]);

        if (animation == nullptr) {
            continue;;
        }

        double animationSpeed = (double)animationComponent["Speed"];

        if (animationSpeed != 0.0) {
            double nextTime = (double)animationComponent["Time"] + animationSpeed * dt;


            if (!(bool)animationComponent["Loop"]) {
                if (nextTime > animation->Duration) {
                    nextTime = animation->Duration;
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationComponent["AnimationName"];
                    m_EventBroker->Publish(e);
                } else if (nextTime < 0) {
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationComponent["AnimationName"];
                    m_EventBroker->Publish(e);
                    nextTime = 0;
                }

                (double&)animationComponent["Speed"] = 0.0;
                
            } else {
                if (nextTime > animation->Duration) {
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationComponent["AnimationName"];
                    m_EventBroker->Publish(e);

                    while(nextTime > animation->Duration) {
                        nextTime -= animation->Duration;
                    }
                } else if (nextTime < 0) {
                    Events::AnimationComplete e;
                    e.Entity = entity;
                    e.Name = (std::string)animationComponent["AnimationName"];
                    m_EventBroker->Publish(e);

                    while (nextTime < 0) {
                        nextTime += animation->Duration;
                    }
                }
            }

            (double&)animationComponent["Time"] = nextTime;
        }
    } 
}

