#include "Common.h"
#include "Core/System.h"

class RaptorCopterSystem : public PureSystem
{
public:
    RaptorCopterSystem(EventBroker* eventBroker)
        : System(eventBroker)
        , PureSystem("RaptorCopter")
    { }

    virtual void UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt) override
    {
        ComponentWrapper& transform = world->GetComponent(component.EntityID, "Transform");
        (glm::vec3&)transform["Orientation"] += (float)(double)component["Speed"] * (float)dt * (glm::vec3)component["Axis"];
    }
};