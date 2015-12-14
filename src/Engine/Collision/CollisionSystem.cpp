#include "Collision/CollisionSystem.h"


void CollisionSystem::Update(World * world, ComponentWrapper & collision, double dt)
{
    m_EventBroker->Process<CollisionSystem>();
    (glm::vec3&)collision["Center"] = (glm::vec3)world->GetComponent(collision.EntityID, "Transform")["Position"];
}

bool CollisionSystem::OnKeyUp(const Events::KeyUp & event)
{
    if (event.KeyCode == GLFW_KEY_Z) {

    }
    return false;
}
