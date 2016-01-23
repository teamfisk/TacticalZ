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

        if (player.HasComponent("Physics")) {
            ComponentWrapper cPhysics = player["Physics"];

            glm::vec3 wishDirection = controller->Movement() * glm::inverse(glm::quat(ori));
            float wishSpeed;
            if (controller->Crouching()) {
                wishSpeed = player["Player"]["CrouchSpeed"];
            } else {
                wishSpeed = player["Player"]["MovementSpeed"];
            }
            glm::vec3& velocity = cPhysics["Velocity"];
            ImGui::Text("velocity: (%f, %f, %f)", velocity.x, velocity.y, velocity.z);
            ImGui::Text("wishDirection: (%f, %f, %f)", wishDirection.x, wishDirection.y, wishDirection.z);
            float currentSpeedProj = glm::dot(velocity, wishDirection);
            float addSpeed = wishSpeed - currentSpeedProj;
            ImGui::Text("currentSpeedProj: %f", currentSpeedProj);
            ImGui::Text("wishSpeed: %f", wishSpeed);
            ImGui::Text("addSpeed: %f", addSpeed);

            if (addSpeed > 0) {
                static float accel = 15.f;
                ImGui::InputFloat("accel", &accel);
                float actualAccel = accel;
                static float surfaceFriction = 1.f;
                ImGui::InputFloat("surfaceFriction", &surfaceFriction);
                float accelerationSpeed = actualAccel * (float)dt * wishSpeed * surfaceFriction;
                accelerationSpeed = glm::min(accelerationSpeed, addSpeed);
                velocity += accelerationSpeed * wishDirection;
                ImGui::Text("velocity: (%f, %f, %f) |%f|", velocity.x, velocity.y, velocity.z, glm::length(velocity));
            }
            //pos += controller->Movement() * glm::inverse(glm::quat(ori)) * speed * (float)dt;

            //glm::vec3& velocity = player["Physics"]["Velocity"];
            //if (controller->Jumping() && !controller->Crouching() && velocity.y == 0.f) {
            //    velocity.y += 4.f;
            //}

            //if (player.HasComponent("AABB")) {
            //    glm::vec3& size = player["AABB"]["Size"];
            //    if (controller->Crouching()) {
            //        size = glm::vec3(1.f, 1.f, 1.f);
            //    } else {
            //        size = glm::vec3(1.f, 1.6f, 1.f);
            //    }
            //}
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

    // Ground friction
    float speed = glm::length(velocity);
        static float groundFriction = 7.f;
    ImGui::InputFloat("groundFriction", &groundFriction);
    if (speed > 0) {
        float drop = speed * groundFriction * (float)dt;
        velocity *= glm::max(speed - drop, 0.f) / speed;
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
