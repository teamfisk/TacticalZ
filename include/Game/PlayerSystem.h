#ifndef PlayerSystem_h__
#define PlayerSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Core/EventBroker.h"
#include "Core/EKeyDown.h"
#include "Core/EKeyUp.h"

struct KeyInput
{
	bool Forward = false;
	bool Left = false;
	bool Back = false;
	bool Right = false;
};

class PlayerSystem : public System
{
public:
	PlayerSystem(EventBroker* eventBroker)
		: System(eventBroker, "Player")
	{ 
		EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &PlayerSystem::OnKeyDown);
		EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &PlayerSystem::OnKeyUp);
	}

	virtual void Update(World* world, ComponentWrapper& player, double dt) override;

private:
	float m_Speed = 5;
	glm::vec3 m_Direction;
	KeyInput input;

	EventRelay<PlayerSystem, Events::KeyDown> m_EKeyDown;
	bool OnKeyDown(const Events::KeyDown &event);
	EventRelay<PlayerSystem, Events::KeyUp> m_EKeyUp;
	bool OnKeyUp(const Events::KeyUp &event);
};

#endif