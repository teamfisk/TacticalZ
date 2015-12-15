#include "PlayerSystem.h"

void PlayerSystem::UpdateComponent(World * world, ComponentWrapper & player, double dt)
{
    (glm::vec3)player["Velocity"] = glm::vec3(0.f);
    if (player["Forward"]) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt) * -1;
    }
    if (player["Left"]) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt) * -1;
    }
    if (player["Back"]) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt);
    }
    if (player["Right"]) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt);
    }

    ComponentWrapper& transform = world->GetComponent(player.EntityID, "Transform");
    (glm::vec3&)transform["Position"] += (glm::vec3)player["Velocity"];
}
