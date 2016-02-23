#include "Systems/ExplosionEffectSystem.h"

void ExplosionEffectSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
	if ((double)component["TimeSinceDeath"] > (double)component["ExplosionDuration"]) {
		(double)component["TimeSinceDeath"] = 0.f;
	}

	//((glm::vec2)component["Velocity"]).x = min(((glm::vec2)component["Velocity"]).x, ((glm::vec2)component["Velocity"]).y);

	(double&)component["TimeSinceDeath"] += dt;

	//if ((bool)component["Pulsate"] == true) {
	//	if ((double)component["TimeSinceDeath"] > (double)component["ExplosionDuration"] * 0.5) {
	//		(bool)component["Reverse"] = true;
	//	}
	//}
}