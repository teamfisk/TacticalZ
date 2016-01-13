#include "Common.h"
#include "Core/System.h"

class RaptorCopterSystem : public PureSystem
{
public:
    RaptorCopterSystem(EventBroker* eventBroker)
        : System(eventBroker)
        , PureSystem("RaptorCopter")
    { }

    virtual void UpdateComponent(World* world, ComponentWrapper& raptorCopter, double dt) override
    {
        ComponentWrapper& transform = world->GetComponent(raptorCopter.EntityID, "Transform");
        (glm::vec3&)transform["Orientation"] += (float)(double)raptorCopter["Speed"] * (float)dt * (glm::vec3)raptorCopter["Axis"];
    }
};