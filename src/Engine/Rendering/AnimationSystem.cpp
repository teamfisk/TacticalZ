#include "Rendering/AnimationSystem.h"

void AnimationSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& animationComponent, double dt)
{
    if(!entity.HasComponent("Model")) {
        return;
    }

    Model* model;
    try {
        model = ResourceManager::Load<::Model, true>(entity["Model"]["Resource"]);
    } catch (const std::exception&) {
        return;
    }
    

    Skeleton* skeleton = model->m_RawModel->m_Skeleton;

    if(skeleton == nullptr) {
        return;
    }


    const Skeleton::Animation* animation = skeleton->GetAnimation(animationComponent["AnimationName"]);

    if(animation != nullptr) {
        double animationSpeed = (double)animationComponent["Speed"];

        if (animationSpeed != 0.0) {
            double nextTime = (double)animationComponent["Time"] + animationSpeed * dt;


            if (!(bool)animationComponent["Loop"] && glm::abs(nextTime) > animation->Duration) {
                (double&)animationComponent["Time"] = glm::sign(nextTime) * animation->Duration;
                (double&)animationComponent["Speed"] = 0.0;
                Events::AnimationComplete e;
                e.Entity = entity;
                e.Name = (std::string)animationComponent["AnimationName"];
                m_EventBroker->Publish(e);
            } else {
                if (glm::abs(nextTime) > animation->Duration) {
                    (double&)animationComponent["Time"] = glm::abs(nextTime) - animation->Duration;
                } else {
                    (double&)animationComponent["Time"] = nextTime;
                }
            }
        }
    }

}

