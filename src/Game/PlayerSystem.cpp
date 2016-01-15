#include "PlayerSystem.h"

void PlayerSystem::UpdateComponent(World * world, ComponentWrapper & player, double dt)
{
    player["Velocity"] = glm::vec3(0.f, 0.f, 0.f);
    if ((bool&)player["Forward"] == true) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt) * -1;

    }
    if ((bool&)player["Left"] == true) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt) * -1;
    }
    if ((bool&)player["Back"] == true) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt);
    }
    if ((bool&)player["Right"] == true) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt);
    }

    if ((glm::vec3)player["Velocity"] != glm::vec3(0.f)) {
        ComponentWrapper& transform = world->GetComponent(player.EntityID, "Transform");
        (glm::vec3&)transform["Position"] += (glm::vec3)player["Velocity"];
    }
}

bool PlayerSystem::OnTouch(const Events::TriggerTouch &event)
{
    LOG_INFO("Player entity %i touched widget (entity %i).", event.Entity, event.Trigger);
    return false;
}

bool PlayerSystem::OnEnter(const Events::TriggerEnter &event)
{
    LOG_INFO("Player entity %i entered widget (entity %i).", event.Entity, event.Trigger);
    return false;
}

bool PlayerSystem::OnLeave(const Events::TriggerLeave &event)
{
    LOG_INFO("Player entity %i left widget (entity %i).", event.Entity, event.Trigger);
    return false;
}
