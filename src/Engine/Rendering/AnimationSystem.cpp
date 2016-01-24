#include "Rendering/AnimationSystem.h"

void AnimationSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& animationComponent, double dt)
{
    if(!entity.HasComponent("Model")) {
        return;
    }

    Model* model = ResourceManager::Load<Model>(entity["Model"]["Resource"]);
    Skeleton* skeleton = model->m_RawModel->m_Skeleton;
    const Skeleton::Animation* animation = skeleton->GetAnimation(animationComponent["Name"]);

    double animationSpeed = (double)animationComponent["Speed"];

    if( animationSpeed != 0.0) {
        double nextTime = (double)animationComponent["Time"] + animationSpeed * dt;


        if (!(bool)animationComponent["Loop"] && glm::abs(nextTime) > animation->Duration) {
            (double&)animationComponent["Time"] = glm::sign(nextTime) * animation->Duration;
            (double&)animationComponent["Speed"] = 0.0;
            Events::AnimationComplete e;
            e.Entity = entity;
            e.Name = (std::string)animationComponent["Name"];
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

