#include "Systems/ExplosionEffectSystem.h"

void ExplosionEffectSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    Field<double> timeSinceDeath = component["TimeSinceDeath"];
    if (timeSinceDeath > (double)component["ExplosionDuration"]) {
        timeSinceDeath = 0.f;
    }
    timeSinceDeath += dt;

    //if ((bool)Component["Gravity"] == true) {
    //    (bool)Component["ExponentialAccelaration"] = false;
    //}
}

