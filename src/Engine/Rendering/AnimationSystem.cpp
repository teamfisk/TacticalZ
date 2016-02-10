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
            continue;;
        }

        double animationSpeed = (double)animationComponent["Speed" + std::to_string(i)];

        if (animationSpeed != 0.0) {
            double nextTime = (double)animationComponent["Time" + std::to_string(i)] + animationSpeed * dt;


            if (!(bool)animationComponent["Loop" + std::to_string(i)]) {
                if (nextTime > animation->Duration) {
                    nextTime = animation->Duration;
                } else if (nextTime < 0) {
                    nextTime = 0;
                }

                (double&)animationComponent["Speed" + std::to_string(i)] = 0.0;
                Events::AnimationComplete e;
                e.Entity = entity;
                e.Name = (std::string)animationComponent["AnimationName" + std::to_string(i)];
                m_EventBroker->Publish(e);
            } else {
                if (nextTime > animation->Duration) {
                    nextTime -= animation->Duration;
                } else if (nextTime < 0) {
                    nextTime += animation->Duration;
                }
            }

            (double&)animationComponent["Time" + std::to_string(i)] = nextTime;
        }
    } 

    //Calculate bone transforms
    if (skeleton != nullptr) {
        std::vector<Skeleton::AnimationData> animations;
        if (entity.HasComponent("Animation")) {
            for (int i = 1; i <= 3; i++) {
                Skeleton::AnimationData animationData;
                animationData.animation = model->m_RawModel->m_Skeleton->GetAnimation(entity["Animation"]["AnimationName" + std::to_string(i)]);
                if (animationData.animation == nullptr) {
                    continue;
                }
                animationData.time = (double)entity["Animation"]["Time" + std::to_string(i)];
                animationData.weight = (double)entity["Animation"]["Weight" + std::to_string(i)];

                animations.push_back(animationData);
            }
        }

        if (entity.HasComponent("AnimationOffset")) {
            Skeleton::AnimationOffset animationOffset;
            animationOffset.animation = skeleton->GetAnimation(entity["AnimationOffset"]["AnimationName"]);
            animationOffset.time = (double)entity["AnimationOffset"]["Time"];
            skeleton->CalculateFrameBones(animations, animationOffset);
        } else {
            skeleton->CalculateFrameBones(animations);
        }
    }
}

