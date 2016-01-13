#include "Common.h"
#include "Core/System.h"

class PlayerMovementSystem : public PureSystem
{
public:
    PlayerMovementSystem(EventBroker* eventBroker)
        : PureSystem(eventBroker, "Player")
    { }

    virtual void UpdateComponent(World* world, ComponentWrapper& player, double dt);
};