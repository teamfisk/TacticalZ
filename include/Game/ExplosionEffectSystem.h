#include "Common.h"
#include "Core/System.h"

class ExplosionEffectSystem : public PureSystem
{
public:
    ExplosionEffectSystem(EventBroker* eventBroker)
        : PureSystem("ExplosionEffect")
    { }

    virtual void  UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt) override
    {

        if ((double)component["TimeSinceDeath"] > (double)component["ExplosionDuration"]) {
            (double)component["TimeSinceDeath"] = 0.f;
        }
        (double&)component["TimeSinceDeath"] += dt;

        //if ((bool)Component["Gravity"] == true) {
        //    (bool)Component["ExponentialAccelaration"] = false;
        //}
    }
};