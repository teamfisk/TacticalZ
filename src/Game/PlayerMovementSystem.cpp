#include "RaptorCopterSystem.h"

void PlayerMovementSystem::UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    ComponentWrapper& transform = world->GetComponent(component.EntityID);
}