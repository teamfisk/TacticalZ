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

        float playerMovementSpeed = player["Player"]["MovementSpeed"];
        float playerCrouchSpeed = player["Player"]["CrouchSpeed"];

        if (player.HasComponent("Physics")) {
            ComponentWrapper cPhysics = player["Physics"];
            //Assault Dash Check
            if (player.HasComponent("DashAbility")) {
                controller->AssaultDashCheck(dt, ((glm::vec3)cPhysics["Velocity"]).y != 0.0f, player["DashAbility"]["CoolDownMaxTimer"]);
            }
            glm::vec3 wishDirection = controller->Movement() * glm::inverse(glm::quat(ori));
            //this makes sure you can only dash in the 4 directions: forw,backw,left,right
            if (controller->AssaultDashDoubleTapped() && controller->Movement().z != 0 && controller->Movement().x != 0) {
                wishDirection = glm::vec3(controller->Movement().x, 0, 0)* glm::inverse(glm::quat(ori));
            }
            float wishSpeed;
            if (controller->Crouching()) {
                wishSpeed = playerCrouchSpeed;
            } else {
                wishSpeed = playerMovementSpeed;
            }
            glm::vec3& velocity = cPhysics["Velocity"];
            ImGui::Text("velocity: (%f, %f, %f)", velocity.x, velocity.y, velocity.z);
            glm::vec3 groundVelocity(0.f, 0.f, 0.f);
            groundVelocity.x = glm::dot(velocity, glm::vec3(1.f, 0.f, 0.f));
            groundVelocity.z = glm::dot(velocity, glm::vec3(0.f, 0.f, 1.f));
            ImGui::Text("groundVelocity: (%f, %f, %f) |%f|", groundVelocity.x, groundVelocity.y, groundVelocity.z, glm::length(wishDirection));
            ImGui::Text("wishDirection: (%f, %f, %f) |%f|", wishDirection.x, wishDirection.y, wishDirection.z, glm::length(wishDirection));
            float currentSpeedProj = glm::dot(groundVelocity, wishDirection);
            float addSpeed = wishSpeed - currentSpeedProj;
            ImGui::Text("currentSpeedProj: %f", currentSpeedProj);
            ImGui::Text("wishSpeed: %f", wishSpeed);
            ImGui::Text("addSpeed: %f", addSpeed);

            if (addSpeed > 0) {
                static float accel = 15.f;
                ImGui::InputFloat("accel", &accel);
                static float airAccel = 0.5f;
                ImGui::InputFloat("airAccel", &airAccel);
                float actualAccel = (velocity.y != 0) ? airAccel : accel;
                static float surfaceFriction = 5.f;
                ImGui::InputFloat("surfaceFriction", &surfaceFriction);
                float accelerationSpeed = actualAccel * (float)dt * wishSpeed * surfaceFriction;
                //if doubleTapped do Assault Dash - but only boost maximum 50.0f
                float doubleTapDashBoost = controller->AssaultDashDoubleTapped() ? 40.0f : 1.0f;
                accelerationSpeed = glm::min(doubleTapDashBoost*glm::min(accelerationSpeed, addSpeed), 50.0f);
                velocity += accelerationSpeed * wishDirection;
                ImGui::Text("velocity: (%f, %f, %f) |%f|", velocity.x, velocity.y, velocity.z, glm::length(velocity));
            }

            //you cant jump and dash at the same time - since there is no friction in the air and we would thus dash much further in the air
            if (!controller->PlayerIsDashing() && controller->Jumping() && !controller->Crouching() && (velocity.y == 0.f || !controller->DoubleJumping())) {
                if (velocity.y == 0.f) {
                    controller->SetDoubleJumping(false);
                } else {
                    controller->SetDoubleJumping(true);
                    Events::DoubleJump e;
                    m_EventBroker->Publish(e);
                }
                velocity.y += 4.f;
            }

            if (player.HasComponent("AABB")) {
                glm::vec3& size = player["AABB"]["Size"];
                if (controller->Crouching()) {
                    size = glm::vec3(1.f, 1.f, 1.f);
                } else {
                    size = glm::vec3(1.f, 1.6f, 1.f);
                }
            }

            // Animations
            EntityWrapper playerModel = player.FirstChildByName("PlayerModel");
            if (playerModel.Valid()) {
                ComponentWrapper cAnimation = playerModel["Animation"];

                float movementLength = glm::length(groundVelocity);
                //TODO: add assault dash animation here
                if (glm::length(controller->Movement()) > 0.f) {
                    if (controller->Crouching()) {
                        cAnimation["Name"] = "Crouch Walk";
                        (double&)cAnimation["Speed"] = 1.f * -glm::sign(controller->Movement().z);
                    } else {
                        cAnimation["Name"] = "Run";
                        (double&)cAnimation["Speed"] = 2.f * -glm::sign(controller->Movement().z);
                    }
                } else {
                    if (controller->Crouching()) {
                        cAnimation["Name"] = "Crouch";
                        (double&)cAnimation["Speed"] = 1.f;
                    } else {
                        cAnimation["Name"] = "Hold Pos";
                        (double&)cAnimation["Speed"] = 1.f;
                    }
                }
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

    // Ground friction
    float speed = glm::length(velocity);
    static float groundFriction = 7.f;
    ImGui::InputFloat("groundFriction", &groundFriction);
    static float airFriction = 0.f;
    ImGui::InputFloat("airFriction", &airFriction);
    float friction = (velocity.y != 0) ? airFriction : groundFriction;
    if (speed > 0) {
        float drop = speed * friction * (float)dt;
        float multiplier = glm::max(speed - drop, 0.f) / speed;
        velocity.x *= multiplier;
        velocity.z *= multiplier;
    }

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
