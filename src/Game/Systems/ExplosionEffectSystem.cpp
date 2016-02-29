#include "Systems/ExplosionEffectSystem.h"

void ExplosionEffectSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    if ((double)component["TimeSinceDeath"] > (double)component["ExplosionDuration"]) {
        (double)component["TimeSinceDeath"] = 0.f;
    }
    (double&)component["TimeSinceDeath"] += dt;

    //if ((bool)Component["Gravity"] == true) {
    //    (bool)Component["ExponentialAccelaration"] = false;
    //}
}

