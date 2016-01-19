#include "Common.h"
#include "GLM.h"
#include "Core/System.h"

class PlayerMovementSystem : public PureSystem
{
public:
    PlayerMovementSystem(EventBroker* eventBroker)
        : System(eventBroker)
        , PureSystem("Player")
    { }

    virtual void UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt);
};