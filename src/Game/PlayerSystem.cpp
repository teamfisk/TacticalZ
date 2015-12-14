#include "PlayerSystem.h"

void PlayerSystem::UpdateComponent(World * world, ComponentWrapper & player, double dt)
{
    if (input.Forward) {
        m_Direction.z = -1;
    } else if (input.Back) {
        m_Direction.z = 1;
    } else {
        m_Direction.z = 0;
    }
    if (input.Left) {
        m_Direction.x = -1;
    } else if (input.Right) {
        m_Direction.x = 1;
    } else {
        m_Direction.x = 0;
    }
    m_EventBroker->Process<PlayerSystem>();
    ComponentWrapper& transform = world->GetComponent(player.EntityID, "Transform");
    (glm::vec3&)player["Velocity"] = m_Speed * float(dt) * m_Direction;
    (glm::vec3&)transform["Position"] += (glm::vec3)player["Velocity"];
}

bool PlayerSystem::OnKeyDown(const Events::KeyDown & event)
{
    if (event.KeyCode == GLFW_KEY_W) {
        input.Forward = true;
    }
    if (event.KeyCode == GLFW_KEY_A) {
        input.Left = true;
    }
    if (event.KeyCode == GLFW_KEY_S) {
        input.Back = true;
    }
    if (event.KeyCode == GLFW_KEY_D) {
        input.Right = true;
    }
    return true;
}

bool PlayerSystem::OnKeyUp(const Events::KeyUp & event)
{
    if (event.KeyCode == GLFW_KEY_W) {
        input.Forward = false;
    }
    if (event.KeyCode == GLFW_KEY_A) {
        input.Left = false;
    }
    if (event.KeyCode == GLFW_KEY_S) {
        input.Back = false;
    }
    if (event.KeyCode == GLFW_KEY_D) {
        input.Right = false;
    }
    return false;
}
