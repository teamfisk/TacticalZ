#include "Common.h"
#include "Core/System.h"

class CoolDeathAnimationSystem : public PureSystem
{
public:
    CoolDeathAnimationSystem(EventBroker* eventBroker)
        : PureSystem(eventBroker, "CoolDeathAnim")
    { }

    virtual void UpdateComponent(World* world, ComponentWrapper& object, double dt) override
    {
        ComponentWrapper& Component = world->GetComponent(object.EntityID, "CoolDeathAnim");

        if ((double)Component["TimeSinceDeath"] > (double)Component["ExplosionDuration"]) {
            (double)Component["TimeSinceDeath"] = 0.f;
        }
        (double&)Component["TimeSinceDeath"] += dt;

        //if ((bool)Component["Gravity"] == true) {
        //    (bool)Component["ExponentialAccelaration"] = false;
        //}
    }
};