#include "Systems/PlayerMovementSystem.h"

void PlayerMovementSystem::UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    ComponentWrapper& cTransform = entity["Transform"];
    if (!entity.HasComponent("Physics")) {
        return;
    }
    ComponentWrapper& cPhysics = entity["Physics"];

    glm::vec3& velocity = cPhysics["Velocity"];
    if (cPhysics["Gravity"]) {
        velocity.y -= 9.82 * dt;
    }

    glm::vec3& position = cTransform["Position"];
    position += velocity * (float)dt;
}