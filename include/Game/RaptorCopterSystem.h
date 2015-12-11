#include "Common.h"
#include "Core/System.h"

class RaptorCopterSystem : public System
{
public:
    RaptorCopterSystem(EventBroker* eventBroker)
        : System(eventBroker, "RaptorCopter")
    { }

    virtual void Update(World* world, ComponentWrapper& raptorCopter, double dt) override
    {
        ComponentWrapper& transform = world->GetComponent(raptorCopter.EntityID, "Transform");
        (glm::vec3&)transform["Orientation"] += (float)(double)raptorCopter["Speed"] * (float)dt * (glm::vec3)raptorCopter["Axis"];
    }
};