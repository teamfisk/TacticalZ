#include "PlayerSystem.h"

void PlayerSystem::UpdateComponent(World * world, ComponentWrapper & player, double dt)
{
    (glm::vec3)player["Velocity"] = glm::vec3(0.f, 0.f, 0.f);
    if (static_cast<bool>(player["Forward"])== true) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt) * -1;
    }
    if (static_cast<bool>(player["Left"]) == true) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt) * -1;
    }
    if (static_cast<bool>(player["Back"]) == true) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt);
    }
    if (static_cast<bool>(player["Right"]) == true) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt);
    }

    if ((glm::vec3)player["Velocity"] != glm::vec3(0.f)) {
        ComponentWrapper& transform = world->GetComponent(player.EntityID, "Transform");
        (glm::vec3&)transform["Position"] += (glm::vec3)player["Velocity"];
    }
}
