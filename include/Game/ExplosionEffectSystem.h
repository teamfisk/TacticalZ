#include "Common.h"
#include "Core/System.h"

class ExplosionEffectSystem : public PureSystem
{
public:
    ExplosionEffectSystem(EventBroker* eventBroker)
        : PureSystem(eventBroker, "ExplosionEffect")
    { }

    virtual void UpdateComponent(World* world, ComponentWrapper& object, double dt) override
    {
        ComponentWrapper& Component = world->GetComponent(object.EntityID, "ExplosionEffect");

        if ((double)Component["TimeSinceDeath"] > (double)Component["ExplosionDuration"]) {
            (double)Component["TimeSinceDeath"] = 0.f;
        }
        (double&)Component["TimeSinceDeath"] += dt;

        //if ((bool)Component["Gravity"] == true) {
        //    (bool)Component["ExponentialAccelaration"] = false;
        //}
    }
};