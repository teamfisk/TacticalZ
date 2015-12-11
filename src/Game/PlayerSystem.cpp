#include "PlayerSystem.h"

void PlayerSystem::Initialize()
{
	EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &PlayerSystem::OnKeyDown);
	EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &PlayerSystem::OnKeyUp);
}

void PlayerSystem::Update(World * world, ComponentWrapper & player, double dt)
{
	m_EventBroker->Process<PlayerSystem>();
	ComponentWrapper& transform = world->GetComponent(player.EntityID, "Transform");
	(glm::vec3&)player["Velocity"] = m_Speed * float(dt) * m_Direction;
	(glm::vec3&)transform["Position"] += (glm::vec3)player["Velocity"];
}

bool PlayerSystem::OnKeyDown(const Events::KeyDown & event)
{
	if (event.KeyCode == GLFW_KEY_P) {
		m_Direction.z == -1;
	}
	if (event.KeyCode == GLFW_KEY_LEFT) {
		m_Direction.x == -1;
	}
	if (event.KeyCode == GLFW_KEY_DOWN) {
		m_Direction.z == 1;
	}
	if (event.KeyCode == GLFW_KEY_RIGHT) {
		m_Direction.x == 1;
	}
	return true;
}

bool PlayerSystem::OnKeyUp(const Events::KeyUp & event)
{
	if (event.KeyCode == GLFW_KEY_UP) {
		m_Direction.z == 0;
	}
	if (event.KeyCode == GLFW_KEY_LEFT) {
		m_Direction.x == 0;
	}
	if (event.KeyCode == GLFW_KEY_DOWN) {
		m_Direction.z == 0;
	}
	if (event.KeyCode == GLFW_KEY_RIGHT) {
		m_Direction.x == 0;
	}
	return false;
}
