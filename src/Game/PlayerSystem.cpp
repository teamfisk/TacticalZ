#include "PlayerSystem.h"

void PlayerSystem::UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    component["Velocity"] = glm::vec3(0.f, 0.f, 0.f);
    if ((bool&)component["Forward"] == true) {
        ((glm::vec3&)component["Velocity"]).z = m_Speed * float(dt) * -1;

    }
    if ((bool&)component["Left"] == true) {
        ((glm::vec3&)component["Velocity"]).x = m_Speed * float(dt) * -1;
    }
    if ((bool&)component["Back"] == true) {
        ((glm::vec3&)component["Velocity"]).z = m_Speed * float(dt);
    }
    if ((bool&)component["Right"] == true) {
        ((glm::vec3&)component["Velocity"]).x = m_Speed * float(dt);
    }

    if ((glm::vec3)component["Velocity"] != glm::vec3(0.f)) {
        ComponentWrapper& transform = world->GetComponent(component.EntityID, "Transform");
        (glm::vec3&)transform["Position"] += (glm::vec3)component["Velocity"];
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