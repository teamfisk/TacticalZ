#include "Common.h"
#include "Core/System.h"

class FloatingEffectSystem : public PureSystem
{
public:
    FloatingEffectSystem(SystemParams params)
        : System(params)
        , PureSystem("FloatingEffect")
    { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt) override
    {
        ComponentWrapper& transform = m_World->GetComponent(component.EntityID, "Transform");
        (double&)component["Time"] += dt;
        //(double)component["Amplitude"] * glm::sin((glm::two_pi<double>() / (double)component["Period"]) *  (double)component["Time"]);
        (glm::vec3&)transform["Position"] = (float)(double)component["Amplitude"] * glm::sin((glm::two_pi<float>() / (float)(double)component["Period"]) *  (float)(double)component["Time"]) * (glm::vec3)component["Axis"];

    }
};