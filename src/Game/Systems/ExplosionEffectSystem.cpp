#include "Systems/ExplosionEffectSystem.h"

void ExplosionEffectSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    Field<double> delay = component["Delay"];
    if (delay > 0) {
        delay = std::max(0.0, delay - dt);
    }

    if (delay <= 0) {
        Field<double> timeSinceDeath = component["TimeSinceDeath"];
        timeSinceDeath += (Field<double>)component["Speed"] * dt;
        if (timeSinceDeath < 0 || timeSinceDeath > (const double&)component["ExplosionDuration"]) {
            timeSinceDeath = 0.0;
        }
    }
}

