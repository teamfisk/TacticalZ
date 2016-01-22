#include "Systems/PlayerMovementSystem.h"

PlayerMovementSystem::PlayerMovementSystem(World* world, EventBroker* eventBroker) 
    : System(world, eventBroker)
    , PureSystem("Player")
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &PlayerMovementSystem::OnPlayerSpawned);
}

PlayerMovementSystem::~PlayerMovementSystem()
{
    for (auto& kv : m_PlayerInputControllers) {
        delete kv.second;
    }
}

void PlayerMovementSystem::Update(double dt)
{
    for (auto& kv : m_PlayerInputControllers) {
        EntityWrapper player = kv.first;
        auto& controller = kv.second;

        if (!player.Valid()) {
            continue;
        }

        EntityWrapper cameraEntity = player.FirstChildByName("Camera");
        if (cameraEntity.Valid()) {
            glm::vec3& cameraOrientation = cameraEntity["Transform"]["Orientation"];
            cameraOrientation.x += controller->Rotation().x;
        }

        ComponentWrapper& cTransform = player["Transform"];
        glm::vec3& ori = cTransform["Orientation"];
        ori.y += controller->Rotation().y;

        glm::vec3& pos = cTransform["Position"];
        pos += controller->Movement() * glm::inverse(glm::quat(ori)) * (float)player["Player"]["MovementSpeed"] * (float)dt;
        
        controller->Reset();
    }
}

void PlayerMovementSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    ComponentWrapper& cTransform = entity["Transform"];
    if (!entity.HasComponent("Physics")) {
        return;
    }
    ComponentWrapper& cPhysics = entity["Physics"];

    glm::vec3& velocity = cPhysics["Velocity"];
    if (cPhysics["Gravity"]) {
        velocity.y -= 9.82f * (float)dt;
    }

    glm::vec3& position = cTransform["Position"];
    position += velocity * (float)dt;
}

bool PlayerMovementSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    // When a player spawns, create an input controller for them
    m_PlayerInputControllers[e.Player] = new FirstPersonInputController<PlayerMovementSystem>(m_EventBroker, e.PlayerID);

    return true;
}
