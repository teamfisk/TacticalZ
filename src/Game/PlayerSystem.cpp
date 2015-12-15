#include "PlayerSystem.h"

void PlayerSystem::UpdateComponent(World * world, ComponentWrapper & player, double dt)
{
    if (player["Forward"]) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt) * -1;
    }
    if (player["Left"]) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt) * -1;
    }
    if (player["Back"]) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt);
    }
    if (player["Right"]) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt);
    }
    ComponentWrapper& transform = world->GetComponent(player.EntityID, "Transform");
    (glm::vec3&)transform["Position"] += (glm::vec3)player["Velocity"];

    //m_EventBroker->Process<PlayerSystem>();

    //// TODO Jag tror inte vi kan göra det vi vill i updaten.
    //// om man lägger till parametrar och tar bort overriden så blir det kanske inte så kul?
    //// aja, lycka till!
    //if (ShouldCreatePlayer) {
    //    int entityID = world->CreateEntity();
    //    ComponentWrapper transform = world->AttachComponent(entityID, "Transform");
    //    transform["Position"] = glm::vec3(-1.5f, 0.f, 0.f);
    //    ComponentWrapper model = world->AttachComponent(entityID, "Model");
    //    model["Resource"] = "Models/Core/UnitSphere.obj";//modelPath; // You fix this :)
    //    model["Color"] = glm::vec4(rand()%255 / 255.f, rand()%255 / 255.f, rand() %255 / 255.f, 1.f);
    //    ShouldCreatePlayer = false;
    //}
}

void PlayerSystem::CreatePlayer(World * world, unsigned int& entityID)
{
    
}

bool PlayerSystem::OnKeyDown(const Events::KeyDown & event)
{
    if (event.KeyCode == GLFW_KEY_W) {
        input.Forward = true;
    }
    if (event.KeyCode == GLFW_KEY_A) {
        input.Left = true;
    }
    if (event.KeyCode == GLFW_KEY_S) {
        input.Back = true;
    }
    if (event.KeyCode == GLFW_KEY_D) {
        input.Right = true;
    }
    return true;
}

bool PlayerSystem::OnKeyUp(const Events::KeyUp & event)
{
    if (event.KeyCode == GLFW_KEY_W) {
        input.Forward = false;
    }
    if (event.KeyCode == GLFW_KEY_A) {
        input.Left = false;
    }
    if (event.KeyCode == GLFW_KEY_S) {
        input.Back = false;
    }
    if (event.KeyCode == GLFW_KEY_D) {
        input.Right = false;
    }
    return false;
}

bool PlayerSystem::OnCreatePlayer(const Events::CreatePlayer & event)
{
    ShouldCreatePlayer = true;
    return false;
}
