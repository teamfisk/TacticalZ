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
            // Limit camera pitch so we don't break our necks
            cameraOrientation.x = glm::clamp(cameraOrientation.x, -glm::half_pi<float>(), glm::half_pi<float>());
        }

        ComponentWrapper& cTransform = player["Transform"];
        glm::vec3& ori = cTransform["Orientation"];
        ori.y += controller->Rotation().y;

        glm::vec3& pos = cTransform["Position"];
        float speed;
        if (controller->Crouching()) {
            speed = player["Player"]["CrouchSpeed"];
        } else {
            speed = player["Player"]["MovementSpeed"];
        }
        pos += controller->Movement() * glm::inverse(glm::quat(ori)) * speed * (float)dt;

        if (player.HasComponent("Physics")) {
            glm::vec3& velocity = player["Physics"]["Velocity"];
            if (controller->Jumping() && !controller->Crouching() && velocity.y == 0.f) {
                velocity.y += 4.f;
            }
        }

        if (player.HasComponent("AABB")) {
            glm::vec3& size = player["AABB"]["Size"];
            if (controller->Crouching()) {
                size = glm::vec3(1.f, 1.f, 1.f);
            } else {
                size = glm::vec3(1.f, 1.6f, 1.f);
            }
        }
        
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
