#include "Systems/PlayerMovementSystem.h"

PlayerMovementSystem::PlayerMovementSystem(SystemParams params)
    : System(params)
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
    updateMovementControllers(dt);
    updateVelocity(dt);
}

void PlayerMovementSystem::updateMovementControllers(double dt)
{
    for (auto& kv : m_PlayerInputControllers) {
        EntityWrapper player = kv.first;
        auto& controller = kv.second;

        if (!player.Valid()) {
            continue;
        }

        // Aim pitch
        EntityWrapper cameraEntity = player.FirstChildByName("Camera");
        if (cameraEntity.Valid()) {
            glm::vec3& cameraOrientation = cameraEntity["Transform"]["Orientation"];
            cameraOrientation.x += controller->Rotation().x;
            // Limit camera pitch so we don't break our necks
            cameraOrientation.x = glm::clamp(cameraOrientation.x, -glm::half_pi<float>(), glm::half_pi<float>());
            // Set third person model aim pitch
            EntityWrapper playerModel = player.FirstChildByName("PlayerModel");
            if (playerModel.Valid()) {
                ComponentWrapper cAnimationOffset = playerModel["AnimationOffset"];
                float pitch = cameraOrientation.x + 0.2;
                double time = (pitch + glm::half_pi<float>()) / glm::pi<float>();
                cAnimationOffset["Time"] = time;
            }
        }

        ComponentWrapper& cTransform = player["Transform"];
        glm::vec3& ori = cTransform["Orientation"];
        ori.y += controller->Rotation().y;

        float playerMovementSpeed = player["Player"]["MovementSpeed"];
        float playerCrouchSpeed = player["Player"]["CrouchSpeed"];
        glm::vec3& wishDirection = player["Player"]["CurrentWishDirection"];

        if (player.HasComponent("Physics")) {
            ComponentWrapper cPhysics = player["Physics"];
            //Assault Dash Check
            if (player.HasComponent("DashAbility")) {
                controller->AssaultDashCheck(dt, ((glm::vec3)cPhysics["Velocity"]).y != 0.0f, player["DashAbility"]["CoolDownMaxTimer"]);
            }
            wishDirection = controller->Movement() * glm::inverse(glm::quat(ori));
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
            if (player.ID == m_LocalPlayer.ID) {
                if (glm::length(wishDirection) == 0) {
                    // If no key is pressed, reset the distance moved since last step.
                    m_DistanceMoved = 0;
                }
            }
            glm::vec3& velocity = cPhysics["Velocity"];
            bool isOnGround = (bool)cPhysics["IsOnGround"];
            //ImGui::Text(isOnGround ? "On ground" : "In air");
            //ImGui::Text("velocity: (%f, %f, %f) |%f|", velocity.x, velocity.y, velocity.z, glm::length(velocity));
            glm::vec3 groundVelocity(0.f, 0.f, 0.f);
            groundVelocity.x = velocity.x;
            groundVelocity.z = velocity.z;
            //ImGui::Text("groundVelocity: (%f, %f, %f) |%f|", groundVelocity.x, groundVelocity.y, groundVelocity.z, glm::length(groundVelocity));
            //ImGui::Text("wishDirection: (%f, %f, %f) |%f|", wishDirection.x, wishDirection.y, wishDirection.z, glm::length(wishDirection));
            float currentSpeedProj = glm::dot(groundVelocity, wishDirection);
            float addSpeed = wishSpeed - currentSpeedProj;
            //ImGui::Text("currentSpeedProj: %f", currentSpeedProj);
            //ImGui::Text("wishSpeed: %f", wishSpeed);
            //ImGui::Text("addSpeed: %f", addSpeed);

            if (addSpeed > 0) {
                static float accel = 15.f;
                ImGui::InputFloat("accel", &accel);
                static float airAccel = 0.5f;
                ImGui::InputFloat("airAccel", &airAccel);
                float actualAccel = isOnGround ? accel : airAccel;
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
            if (!controller->PlayerIsDashing() && controller->Jumping() && !controller->Crouching() && (isOnGround || !controller->DoubleJumping())) {
                (bool)cPhysics["IsOnGround"] = false;
                if (isOnGround) {
                    controller->SetDoubleJumping(false);
                } else {
                    if (IsClient) {
                        //put a hexagon at the players feet
                        auto hexagonEffect = ResourceManager::Load<EntityFile>("Schema/Entities/DoubleJumpHexagon.xml");
                        EntityFileParser parser(hexagonEffect);
                        EntityID hexagonEffectID = parser.MergeEntities(m_World);
                        EntityWrapper hexagonEW = EntityWrapper(m_World, hexagonEffectID);
                        hexagonEW["Transform"]["Position"] = (glm::vec3)player["Transform"]["Position"];
                        controller->SetDoubleJumping(true);
                        Events::DoubleJump e;
                        m_EventBroker->Publish(e);
                    }
                }
                velocity.y = 4.f;
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
                std::string& animationName1 = cAnimation["AnimationName1"];
                std::string& animationName2 = cAnimation["AnimationName2"];
                double& animationTime1 = cAnimation["Time1"];
                double& animationTime2 = cAnimation["Time2"];
                double& animationSpeed1 = cAnimation["Speed1"];
                double& animationSpeed2 = cAnimation["Speed2"];
                double& animationWeight1 = cAnimation["Weight1"];
                double& animationWeight2 = cAnimation["Weight2"];

                float movementLength = glm::length(groundVelocity);
                //TODO: add assault dash animation here
                if (glm::length(controller->Movement()) > 0.f) {
                    double forwardMovement = controller->Movement().z;
                    double strafeMovement = controller->Movement().x;

                    if (controller->Crouching() && animationName1 != "CrouchWalk") {
                        animationName1 = "CrouchWalk";
                        animationSpeed1 = 1.0 * -glm::sign(controller->Movement().z);
                    } else {
                        if (glm::abs(forwardMovement) > 0) {
                            if (animationName1 != "Run") {
                                animationName1 = "Run";
                                if (animationName2 == "StrafeLeft" || animationName2 == "StrafeRight") {
                                    animationTime1 = animationTime2;
                                } else {
                                    animationTime1 = 0.0;
                                }
                            }
                            animationSpeed1 = 2.f * -glm::sign(forwardMovement);
                        }

                        if (glm::abs(strafeMovement) > 0) {
                            if (animationName2 != "StrafeLeft" && animationName2 != "StrafeRight") {
                                if (strafeMovement < 0) {
                                    animationName2 = "StrafeLeft";
                                }
                                if (strafeMovement > 0) {
                                    animationName2 = "StrafeRight";
                                }
                                if (animationName1 == "Run") {
                                    animationTime2 = animationTime1;
                                } else {
                                    animationTime2 = 0.0;
                                }
                            }
                            animationSpeed2 = 2.f * glm::abs(strafeMovement);
                        }

                        double strafeWeight = glm::abs(strafeMovement) / (glm::abs(forwardMovement) + glm::abs(strafeMovement));
                        animationWeight2 = strafeWeight;
                        animationWeight1 = 1.0 - strafeWeight;
                    }
                } else {
                    if (controller->Crouching()) {
                        animationName1 = "Crouch";
                        animationName2 = "";
                        animationSpeed1 = 1.0;
                        animationSpeed2 = 0.0;
                        animationWeight1 = 1.0;
                        animationWeight2 = 0.0;
                    } else {
                        animationName1 = "Idle";
                        animationName2 = "";
                        animationSpeed1 = 1.f;
                        animationSpeed2 = 0.0;
                        animationWeight1 = 1.0;
                        animationWeight2 = 0.0;
                        //cAnimation["AnimationName2"] = "Idle";
                    }
                }
            }
        }

        controller->Reset();
    }
    playerStep(dt);
}


void PlayerMovementSystem::updateVelocity(double dt)
{
    // Only apply velocity to local player
    if (!LocalPlayer.Valid()) {
        return;
    }

    ComponentWrapper& cTransform = LocalPlayer["Transform"];
    ComponentWrapper& cPhysics = LocalPlayer["Physics"];
    glm::vec3& velocity = cPhysics["Velocity"];
    bool isOnGround = (bool)cPhysics["IsOnGround"];

    // Ground friction
    float speed = glm::length(velocity);
    static float groundFriction = 7.f;
    ImGui::InputFloat("groundFriction", &groundFriction);
    static float airFriction = 2.f;

    ImGui::InputFloat("airFriction", &airFriction);
    float friction = isOnGround ? groundFriction : airFriction;
    if (speed > 0) {
        float drop = speed * friction * (float)dt;
        float multiplier = glm::max(speed - drop, 0.f) / speed;
        velocity.x *= multiplier;
        velocity.z *= multiplier;
    }

    // Gravity
    if (cPhysics["Gravity"]) {
        velocity.y -= 9.82f * (float)dt;
    }

    glm::vec3& position = cTransform["Position"];
    position += velocity * (float)dt;
}

void PlayerMovementSystem::playerStep(double dt)
{
    if (!m_LocalPlayer.Valid()) {
        return;
    }
    // Position of the local player, used see how far a player has moved.
    glm::vec3 pos = (glm::vec3)m_World->GetComponent(m_LocalPlayer.ID, "Transform")["Position"];
    // Used to see if a player is airborne.
    bool grounded = (bool)m_World->GetComponent(m_LocalPlayer.ID, "Physics")["IsOnGround"];
    m_DistanceMoved += glm::length(pos - m_LastPosition);
    // Set the last position for next iteration
    m_LastPosition = pos;
    if (m_DistanceMoved > m_PlayerStepLength && grounded) {
        // Player moved a step's distance
        // Create footstep sound
        Events::PlaySoundOnEntity e;
        e.EmitterID = m_LocalPlayer.ID;
        e.FilePath = m_LeftFoot ? "Audio/footstep/footstep2.wav" : "Audio/footstep/footstep3.wav";
        m_LeftFoot = !m_LeftFoot;
        m_EventBroker->Publish(e);
        m_DistanceMoved = 0.f;
    }
}

bool PlayerMovementSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    // When a player spawns, create an input controller for them
    m_PlayerInputControllers[e.Player] = new FirstPersonInputController<PlayerMovementSystem>(m_EventBroker, e.PlayerID);
    if (e.PlayerID == -1) {
        // Keep track of the local player
        m_LocalPlayer = e.Player;
    }
    return true;
}
