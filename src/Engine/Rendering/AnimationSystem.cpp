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
/*

    ImGui::SliderFloat("Angle", &angle, -100.f, 100.f);
    {
        int id = skeleton->GetBoneID("Spine_3");
        auto it = skeleton->Bones.find(id);
        if (it != skeleton->Bones.end()) {
            it->second->ModificationMatrix = glm::mat4(glm::quat(glm::vec3(glm::radians(angle), 0.f, 0.f)));
        }
    }
*/


    const Skeleton::Animation* animation = skeleton->GetAnimation(animationComponent["AnimationName"]);

    if (animation == nullptr) {
        return;
    }

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
    

    



   /* {
        int id = skeleton->GetBoneID("Spine_2");
        auto it = skeleton->Bones.find(id);
        if (it != skeleton->Bones.end()) {
            it->second->ModificationMatrix = glm::mat4(glm::quat(glm::vec3(glm::radians(angle/4.f), 0.f, 0.f)));
        }
    }
    {
        int id = skeleton->GetBoneID("R_Shoulder");
        auto it = skeleton->Bones.find(id);
        if (it != skeleton->Bones.end()) {
            it->second->ModificationMatrix = glm::mat4(glm::quat(glm::vec3(glm::radians(angle/2.f), 0.f, 0.f)));
        }
    }
    {
        int id = skeleton->GetBoneID("L_Shoulder");
        auto it = skeleton->Bones.find(id);
        if (it != skeleton->Bones.end()) {
            it->second->ModificationMatrix = glm::mat4(glm::quat(glm::vec3(glm::radians(angle/2.f), 0.f, 0.f)));
        }
    }*/

/*
    if (entity.HasComponent("Player")) {
        EntityWrapper cameraEntity = entity.FirstChildByName("Camera");
        if (cameraEntity.Valid()) {
            glm::vec3& cameraOrientation = cameraEntity["Transform"]["Orientation"];

            
        }
    }*/
    
}

