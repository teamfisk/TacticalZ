#include "Common.h"
#include "GLM.h"
#include "Core/System.h"

class PlayerMovementSystem : public PureSystem
{
public:
    PlayerMovementSystem(World* world, EventBroker* eventBroker)
        : System(world, eventBroker)
        , PureSystem("Player")
    { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt);
};