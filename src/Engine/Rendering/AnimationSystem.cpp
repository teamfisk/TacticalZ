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

    for (int i = 1; i <= 3; i++) {
        const Skeleton::Animation* animation = skeleton->GetAnimation(animationComponent["AnimationName" + std::to_string(i)]);

        if (animation == nullptr) {
            return;
        }

        double animationSpeed = (double)animationComponent["Speed" + std::to_string(i)];

        if (animationSpeed != 0.0) {
            double nextTime = (double)animationComponent["Time" + std::to_string(i)] + animationSpeed * dt;


            if (!(bool)animationComponent["Loop" + std::to_string(i)] && glm::abs(nextTime) > animation->Duration) {
                (double&)animationComponent["Time" + std::to_string(i)] = glm::sign(nextTime) * animation->Duration;
                (double&)animationComponent["Speed" + std::to_string(i)] = 0.0;
                Events::AnimationComplete e;
                e.Entity = entity;
                e.Name = (std::string)animationComponent["AnimationName" + std::to_string(i)];
                m_EventBroker->Publish(e);
            } else {
                if (glm::abs(nextTime) > animation->Duration) {
                    (double&)animationComponent["Time" + std::to_string(i)] = glm::abs(nextTime) - animation->Duration;
                } else {
                    (double&)animationComponent["Time" + std::to_string(i)] = nextTime;
                }
            }
        }
    } 
}

