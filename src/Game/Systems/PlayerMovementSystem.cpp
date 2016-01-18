#include "Systems/PlayerMovementSystem.h"

void PlayerMovementSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    ComponentWrapper& cTransform = entity["Transform"];
    if (!entity.HasComponent("Physics")) {
        return;
    }
    ComponentWrapper& cPhysics = entity["Physics"];

    glm::vec3& velocity = cPhysics["Velocity"];
    velocity.y -= 9.82 * dt;

    glm::vec3& position = cTransform["Position"];
    position += velocity * (float)dt;
}