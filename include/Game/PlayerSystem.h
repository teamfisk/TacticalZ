#ifndef PlayerSystem_h__
#define PlayerSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Common.h"
#include "Core/System.h"

class PlayerSystem : public PureSystem
{
public:
    PlayerSystem(EventBroker* eventBroker)
        : PureSystem(eventBroker, "Player")
    { }
    virtual void UpdateComponent(World* world, ComponentWrapper& player, double dt) override;
private:
    float m_Speed = 5;
};

#endif