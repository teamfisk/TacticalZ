#include "Systems/ExplosionEffectSystem.h"

void ExplosionEffectSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    double& delay = (double)component["Delay"];
    if (delay > 0) {
        delay = std::max(0.0, delay - dt);
    }

    if (delay <= 0) {
        double& timeSinceDeath = component["TimeSinceDeath"];
        timeSinceDeath += (double)component["Speed"] * dt;
        if (timeSinceDeath < 0 || timeSinceDeath > (double)component["ExplosionDuration"]) {
            timeSinceDeath = 0.0;
        }
    }
}

