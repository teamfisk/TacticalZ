#include "Systems/PlayerMovementSystem.h"

PlayerMovementSystem::PlayerMovementSystem(SystemParams params)
    : System(params)
    , m_SprintEffectTimer(0.f)
    , m_DashEffectTimer(0.f)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &PlayerMovementSystem::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_EDoubleJump, &PlayerMovementSystem::OnDoubleJump);
    EVENT_SUBSCRIBE_MEMBER(m_EDashAbility, &PlayerMovementSystem::OnDashAbility);
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
    // Only do physics calculations on client and only for themselves.
    if (IsClient) {
        if (LocalPlayer.Valid()){
            updateVelocity(LocalPlayer, dt);
        }
        //m_SprintEffectTimer += dt;
        //if (m_SprintEffectTimer < 0.016f) {
        //    return;
        //}
        //m_SprintEffectTimer = 0.f;
        //auto pool = m_World->GetComponents("SprintAbility");
        //if (pool == nullptr) {
        //    return;
        //}
        //for (auto cSprint : *pool) {
        //    if (cSprint.EntityID != LocalPlayer.ID && (bool)cSprint["Active"]) {
        //        // Spawn one afterimage for each player that sprints.
        //        EntityWrapper player(m_World, cSprint.EntityID);
        //        auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/SprintEffect.xml");
        //        EntityWrapper sprintEffect = entityFile->MergeInto(m_World);
        //        auto playerModel = player.FirstChildByName("PlayerModel");
        //        if (!playerModel.Valid()) {
        //            continue;
        //        }
        //        if (!playerModel.HasComponent("Model")) {
        //            continue;
        //        }
        //        if (!playerModel.HasComponent("Animation")) {
        //            continue;
        //        }
        //        auto playerEntityModel = playerModel["Model"];
        //        auto playerEntityAnimation = playerModel["Animation"];
        //        playerEntityModel.Copy(sprintEffect["Model"]);
        //        playerEntityAnimation.Copy(sprintEffect["Animation"]);
        //        sprintEffect["ExplosionEffect"]["EndColor"] = (glm::vec4)playerEntityModel["Color"];
        //        ((Field<glm::vec4>)sprintEffect["ExplosionEffect"]["EndColor"]).w(0.f);
        //        sprintEffect["Animation"]["Speed1"] = 0.0;
        //        sprintEffect["Animation"]["Speed2"] = 0.0;
        //        sprintEffect["Animation"]["Speed3"] = 0.0;
        //        sprintEffect["Transform"]["Position"] = (glm::vec3)player["Transform"]["Position"];
        //        sprintEffect["Transform"]["Orientation"] = (glm::vec3)player["Transform"]["Orientation"];
        //    }
        //}
        m_DashEffectTimer += dt;
        if (m_DashEffectTimer < 0.075f) {
            return;
        }
        m_DashEffectTimer = 0.f;
        auto pool = m_World->GetComponents("DashAbility");
        if (pool == nullptr) {
            return;
        }
        for (auto cDash : *pool) {
            if (cDash.EntityID != LocalPlayer.ID && (double)cDash["CoolDownMaxTimer"] - (double)cDash["CoolDownTimer"] < 0.5) {
                // Spawn one afterimage for each player that Dashes.
                EntityWrapper player(m_World, cDash.EntityID);
                auto playerModel = player.FirstChildByName("PlayerModel");
                if (!playerModel.Valid()) {
                    continue;
                }
                if (!playerModel.HasComponent("Model")) {
                    continue;
                }
                EntityWrapper dashEffect = playerModel.Clone();
                for (auto& cAnim : dashEffect.ChildrenWithComponent("Animation")) {
                    cAnim["Animation"]["Play"] = false;
                }
                const double fadeTime = 0.5f;
                for (auto& cModel : dashEffect.ChildrenWithComponent("Model")) {
                    EntityWrapper e(m_World, cModel.ID);
                    e.AttachComponent("Lifetime");
                    e.AttachComponent("Fade");
                    e["Fade"]["Loop"] = false;
                    e["Fade"]["FadeTime"] = fadeTime;
                    e["Fade"]["Time"] = fadeTime;
                    e["Lifetime"]["Lifetime"] = fadeTime;
                }
                dashEffect.AttachComponent("Lifetime");
                dashEffect.AttachComponent("Fade");
                dashEffect["Fade"]["Loop"] = false;
                dashEffect["Fade"]["FadeTime"] = fadeTime;
                dashEffect["Fade"]["Time"] = fadeTime;
                dashEffect["Lifetime"]["Lifetime"] = fadeTime;
                dashEffect["Transform"]["Position"] = (glm::vec3)player["Transform"]["Position"];
                dashEffect["Transform"]["Orientation"] = (glm::vec3)player["Transform"]["Orientation"];
            }
        }
    }
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
            Field<glm::vec3> cameraOrientation = cameraEntity["Transform"]["Orientation"];
            cameraOrientation.x(cameraOrientation.x() + controller->Rotation().x);
            // Limit camera pitch so we don't break our necks
            cameraOrientation.x(glm::clamp(cameraOrientation.x(), -glm::half_pi<float>(), glm::half_pi<float>()));

            float pitch = cameraOrientation.x();
            double time = ((pitch + glm::half_pi<float>()) / glm::pi<float>());

            // Set third person model aim pitch
            EntityWrapper playerModel = player.FirstChildByName("PlayerModel");
            if (playerModel.Valid()) {
                setAim(playerModel, "SidearmWeapon", time);
                setAim(playerModel, "AssaultWeapon", time);
                setAim(playerModel, "DefenderWeapon", time);
            }
        }

        ComponentWrapper& cTransform = player["Transform"];
        Field<glm::vec3> ori = cTransform["Orientation"];
        ori.y(ori.y() + controller->Rotation().y);

        float playerMovementSpeed = player["Player"]["MovementSpeed"];
        float playerCrouchSpeed = player["Player"]["CrouchSpeed"];
        Field<glm::vec3> wishDirection = player["Player"]["CurrentWishDirection"];
        auto playerBoostAssaultEntity = player.FirstChildByName("BoostAssault");
        if (playerBoostAssaultEntity.Valid()) {
            playerMovementSpeed *= (double)playerBoostAssaultEntity["BoostAssault"]["StrengthOfEffect"];
            playerCrouchSpeed *= (double)playerBoostAssaultEntity["BoostAssault"]["StrengthOfEffect"];
        }
        bool sniperSprinting = false;
        if (player.HasComponent("SprintAbility")) {
            (bool)player["SprintAbility"]["Active"] = controller->SpecialAbilityKeyDown();
            if (controller->SpecialAbilityKeyDown()) {
                playerMovementSpeed *= (double)player["SprintAbility"]["StrengthOfEffect"];
                playerCrouchSpeed *= (double)player["SprintAbility"]["StrengthOfEffect"];
                sniperSprinting = true;
            }
        }

        if (player.HasComponent("Physics")) {
            ComponentWrapper cPhysics = player["Physics"];
            //Assault Dash Check
            if (player.HasComponent("DashAbility")) {
                Field<double> coolDownTimer = player["DashAbility"]["CoolDownTimer"];
                controller->AssaultDashCheck(dt, ((glm::vec3)cPhysics["Velocity"]).y != 0.0f, player["DashAbility"]["CoolDownMaxTimer"], coolDownTimer, player.ID);
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
                if (glm::length((glm::vec3)wishDirection) == 0) {
                    // If no key is pressed, reset the distance moved since last step.
                    m_DistanceMoved = 0;
                }
            }
            Field<glm::vec3> velocity = cPhysics["Velocity"];
            bool isOnGround = (bool)cPhysics["IsOnGround"];
            //ImGui::Text(isOnGround ? "On ground" : "In air");
            //ImGui::Text("velocity: (%f, %f, %f) |%f|", velocity.x, velocity.y, velocity.z, glm::length(velocity));
            glm::vec3 groundVelocity(0.f, 0.f, 0.f);
            groundVelocity.x = velocity.x();
            groundVelocity.z = velocity.z();
            //ImGui::Text("groundVelocity: (%f, %f, %f) |%f|", groundVelocity.x, groundVelocity.y, groundVelocity.z, glm::length(groundVelocity));
            //ImGui::Text("wishDirection: (%f, %f, %f) |%f|", wishDirection.x, wishDirection.y, wishDirection.z, glm::length(wishDirection));
            float currentSpeedProj = glm::dot(groundVelocity, (glm::vec3)wishDirection);
            float addSpeed = wishSpeed - currentSpeedProj;
            //ImGui::Text("currentSpeedProj: %f", currentSpeedProj);
            //ImGui::Text("wishSpeed: %f", wishSpeed);
            //ImGui::Text("addSpeed: %f", addSpeed);

            if (addSpeed > 0) {
                static float accel = 15.f;
#ifdef DEBUG
                ImGui::InputFloat("accel", &accel);
#endif
                static float airAccel = 0.5f;
#ifdef DEBUG
                ImGui::InputFloat("airAccel", &airAccel);
#endif
                float actualAccel = isOnGround ? accel : airAccel;
                static float surfaceFriction = 5.f;
#ifdef DEBUG
                ImGui::InputFloat("surfaceFriction", &surfaceFriction);
#endif
                float accelerationSpeed = actualAccel * (float)dt * wishSpeed * surfaceFriction;
                //if doubleTapped do Assault Dash - but only boost maximum 50.0f
                float doubleTapDashBoost = controller->AssaultDashDoubleTapped() ? 40.0f : 1.0f;
                accelerationSpeed = glm::min(doubleTapDashBoost*glm::min(accelerationSpeed, addSpeed), 50.0f);
                //if player has Boost from an Assault class, accelerate the player faster
                if (playerBoostAssaultEntity.Valid()) {
                    accelerationSpeed *= (double)playerBoostAssaultEntity["BoostAssault"]["StrengthOfEffect"];
                }
                if (sniperSprinting) {
                    accelerationSpeed *= (double)player["SprintAbility"]["StrengthOfEffect"];
                }
                velocity += accelerationSpeed * (glm::vec3)wishDirection;
#ifdef DEBUG
                ImGui::Text("velocity: (%f, %f, %f) |%f|", velocity.x(), velocity.y(), velocity.z(), glm::length((glm::vec3)velocity));
#endif
            }

            if (isOnGround) {
                controller->SetDoubleJumping(false);
            }
            //If player presses Jump and is not crouching.
            if (controller->Jumping() && !controller->Crouching()) {
                if (isOnGround) {
                    (bool)cPhysics["IsOnGround"] = false;
                    velocity.y(player["Player"]["JumpSpeed"]);


                    if (player.Valid()) {
                        EntityWrapper playerModel = player.FirstChildByName("PlayerModel");
                        if (playerModel.Valid()) {
                            {
                                Events::AutoAnimationBlend aeb;
                                aeb.Duration = 0.15;
                                aeb.NodeName = "Jump";
                                aeb.RootNode = playerModel;
                                aeb.Start = true;
                                aeb.Restart = true;
                                m_EventBroker->Publish(aeb);
                            }
                            {
                                Events::AutoAnimationBlend aeb;
                                aeb.Duration = 0.25;
                                aeb.NodeName = "MovementBlend";
                                aeb.RootNode = playerModel;
                                aeb.Start = true;
                                aeb.Restart = false;
                                aeb.AnimationEntity = playerModel.FirstChildByName("Jump");
                                m_EventBroker->Publish(aeb);
                            }
                        }
                    }


                } else if (player.HasComponent("DoubleJump") && !controller->DoubleJumping()) {
                    //Enter here if player can double jump and is doing so.
                    (bool)cPhysics["IsOnGround"] = false;
                    velocity.y(player["DoubleJump"]["DoubleJumpSpeed"]);

                    if (player.Valid()) {
                        EntityWrapper playerModel = player.FirstChildByName("PlayerModel");
                        if (playerModel.Valid()) {
                            {
                                Events::AutoAnimationBlend aeb;
                                aeb.Duration = 0.15;
                                aeb.NodeName = "Jump";
                                aeb.RootNode = playerModel;
                                aeb.Start = true;
                                aeb.Restart = true;
                                m_EventBroker->Publish(aeb);
                            }
                            {
                                Events::AutoAnimationBlend aeb;
                                aeb.Duration = 0.25;
                                aeb.NodeName = "MovementBlend";
                                aeb.RootNode = playerModel;
                                aeb.Start = true;
                                aeb.Restart = false;
                                aeb.AnimationEntity = playerModel.FirstChildByName("BlendTreeLower").FirstChildByName("Jump");
                                m_EventBroker->Publish(aeb);
                            }
                        }
                    }

                    // If IsServer and network is off this will not work
                    if (IsClient) {
                        //put a hexagon at the players feet
                        spawnHexagon(player);
                        controller->SetDoubleJumping(true);
                        // Publish event for client to listen to
                        Events::DoubleJump e;
                        e.entityID = player.ID;
                        m_EventBroker->Publish(e);
                    }
                }
            }

            if (player.HasComponent("AABB")) {
                Field<glm::vec3> size = player["AABB"]["Size"];
                if (controller->Crouching()) {
                    size = glm::vec3(1.f, 1.f, 1.f);
                } else {
                    size = glm::vec3(1.f, 1.6f, 1.f);
                    if (controller->CrouchingLastFrame() && isOnGround) {
                        // The collision should resolve this anyway, but 
                        // this is more reliable, since the box gets larger.
                        Field<glm::vec3> pos = cTransform["Position"];
                        pos.y(pos.y() + 0.3f);
                    }
                }
            }

            // TODO: Animations
        }
        playerStep(dt, player);
        controller->Reset();
    }
}


void PlayerMovementSystem::updateVelocity(EntityWrapper player, double dt)
{
    // Only apply velocity to local player
    ComponentWrapper& cTransform = player["Transform"];
    ComponentWrapper& cPhysics = player["Physics"];
    Field<glm::vec3> velocity = cPhysics["Velocity"];
    bool isOnGround = (bool)cPhysics["IsOnGround"];

    // Ground friction
    float speed = glm::length((glm::vec3)velocity);
    static float groundFriction = 7.f;
#ifdef DEBUG
    ImGui::InputFloat("groundFriction", &groundFriction);
#endif
    static float airFriction = 2.f;

#ifdef DEBUG
    ImGui::InputFloat("airFriction", &airFriction);
#endif
    float friction = isOnGround ? groundFriction : airFriction;
    if (speed > 0) {
        float drop = speed * friction * (float)dt;
        float multiplier = glm::max(speed - drop, 0.f) / speed;
        velocity.x(velocity.x() * multiplier);
        velocity.z(velocity.z() * multiplier);
    }

    // Gravity
    if (cPhysics["Gravity"]) {
        velocity.y(velocity.y() - (9.82f * (float)dt));
    }

    Field<glm::vec3> position = cTransform["Position"];
    position += velocity * (float)dt;
}


void PlayerMovementSystem::setAim(EntityWrapper root, std::string weaponNodeName, double time)
{
    if (root.Valid()) {
        EntityWrapper blendTreeUpper = root.FirstChildByName("BlendTreeUpper");
        if (blendTreeUpper.Valid()) {
            EntityWrapper weapon = blendTreeUpper.FirstChildByName(weaponNodeName);
            if(weapon.Valid()) {
                EntityWrapper aim = weapon.FirstChildByName("Aim");
                if (aim.Valid()) {
                    if (aim.HasComponent("Animation")) {
                        (Field<double>)aim["Animation"]["Time"] = time;
                    }
                }
            }
        }
    }
}

void PlayerMovementSystem::playerStep(double dt, EntityWrapper player)
{
    // Position of the local player, used see how far a player has moved.
    if(!IsClient) {
        return;
    }
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
        e.Emitter = m_LocalPlayer;
        e.FilePath = m_LeftFoot ? "Audio/Footstep/Footstep2.wav" : "Audio/Footstep/Footstep3.wav";
        m_LeftFoot = !m_LeftFoot;
        m_EventBroker->Publish(e);
        m_DistanceMoved = 0.f;
    }
}

bool PlayerMovementSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    // When a player spawns, create an input controller for them
    m_PlayerInputControllers[e.Player] = new FirstPersonInputController<PlayerMovementSystem>(m_EventBroker, e.PlayerID, e.Player);
    if (e.PlayerID == -1) {
        // Keep track of the local player
        m_LocalPlayer = e.Player;
    }
    return true;
}

bool PlayerMovementSystem::OnDoubleJump(Events::DoubleJump & e)
{
    // If entity does not exist, exit
    if (!EntityWrapper(m_World, e.entityID).Valid()) {
        return false;
    }
    // If entity IsLocalPlayer, exit
    if (e.entityID == m_LocalPlayer.ID) {
        return false;
    }
    spawnHexagon(EntityWrapper(m_World, e.entityID));
    return true;
}

void PlayerMovementSystem::spawnHexagon(EntityWrapper target)
{
    //put a hexagon at the entitys... feet?
    auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/DoubleJumpHexagon.xml");
    EntityWrapper hexagonEW = entityFile->MergeInto(m_World);
    hexagonEW["Transform"]["Position"] = (glm::vec3)target["Transform"]["Position"];
}

bool PlayerMovementSystem::OnDashAbility(Events::DashAbility & e)
{
    EntityWrapper eventPlayer(m_World, e.Player);
    if (!eventPlayer.Valid()){// || IsServer || eventPlayer.ID == LocalPlayer.ID) {
        return false;
    }

//    auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/DashEffect.xml");
  //  EntityWrapper dashEffect = entityFile->MergeInto(m_World);
    EntityWrapper playerModel = eventPlayer.FirstChildByName("PlayerModel");

    for (auto& kv : m_PlayerInputControllers) {
        EntityWrapper player = kv.first;
        if(player != eventPlayer) {
            continue;
        }


        auto& controller = kv.second;

        if (!player.Valid()) {
            continue;
        }

        if (player.Valid()) {
            EntityWrapper playerModel = player.FirstChildByName("PlayerModel");
            if (playerModel.Valid()) {
                if (glm::abs(controller->Movement().x) > glm::abs(controller->Movement().z)) {
                    if (controller->Movement().x > 0) {
                        {
                            Events::AutoAnimationBlend aeb;
                            aeb.Duration = 0.1;
                            aeb.NodeName = "DashRight";
                            aeb.RootNode = playerModel;
                            aeb.Restart = true;
                            aeb.Start = true;
                            m_EventBroker->Publish(aeb);
                        }
                        {
                            Events::AutoAnimationBlend aeb;
                            aeb.Duration = 0.2;
                            aeb.NodeName = "MovementBlend";
                            aeb.RootNode = playerModel;
                            aeb.Start = true;
                            aeb.Restart = false;
                            aeb.AnimationEntity = playerModel.FirstChildByName("DashRight");
                            m_EventBroker->Publish(aeb);
                        }
                    } else {
                        {
                            Events::AutoAnimationBlend aeb;
                            aeb.Duration = 0.1;
                            aeb.NodeName = "DashLeft";
                            aeb.RootNode = playerModel;
                            aeb.Restart = true;
                            aeb.Start = true;
                            m_EventBroker->Publish(aeb);
                        }
                        {
                            Events::AutoAnimationBlend aeb;
                            aeb.Duration = 0.2;
                            aeb.NodeName = "MovementBlend";
                            aeb.RootNode = playerModel;
                            aeb.Start = true;
                            aeb.Restart = false;
                            aeb.AnimationEntity = playerModel.FirstChildByName("DashLeft");
                            m_EventBroker->Publish(aeb);
                        }
                    }
                } else {
                    if (controller->Movement().z < 0) {
                        {
                            Events::AutoAnimationBlend aeb;
                            aeb.Duration = 0.1;
                            aeb.NodeName = "DashForward";
                            aeb.RootNode = playerModel;
                            aeb.Restart = true;
                            aeb.Start = true;
                            m_EventBroker->Publish(aeb);
                        }
                        {
                            Events::AutoAnimationBlend aeb;
                            aeb.Duration = 0.2;
                            aeb.NodeName = "MovementBlend";
                            aeb.RootNode = playerModel;
                            aeb.Start = true;
                            aeb.Restart = false;
                            aeb.AnimationEntity = playerModel.FirstChildByName("DashForward");
                            m_EventBroker->Publish(aeb);
                        }
                    } else {
                        {
                            Events::AutoAnimationBlend aeb;
                            aeb.Duration = 0.1;
                            aeb.NodeName = "DashBackward";
                            aeb.RootNode = playerModel;
                            aeb.Restart = true;
                            aeb.Start = true;
                            m_EventBroker->Publish(aeb);
                        }
                        {
                            Events::AutoAnimationBlend aeb;
                            aeb.Duration = 0.2;
                            aeb.NodeName = "MovementBlend";
                            aeb.RootNode = playerModel;
                            aeb.Start = true;
                            aeb.Restart = false;
                            aeb.AnimationEntity = playerModel.FirstChildByName("DashBackward");
                            m_EventBroker->Publish(aeb);
                        }
                    }
                }

/*
                EntityWrapper dashEffectModel;
                dashEffectModel = playerModel.Clone();
                player["Transform"].Copy(dashEffectModel["Transform"]);


                dashEffectModel.AttachComponent("ExplosionEffect");
                dashEffectModel["ExplosionEffect"]["EndColor"] = (glm::vec4)playerModel["Model"]["Color"];
                ((Field<glm::vec4>)dashEffectModel["ExplosionEffect"]["EndColor"]).w = 0.f;
                (Field<double>)dashEffectModel["ExplosionEffect"]["ExplosionDuration"] = dashEffect["Lifetime"]["Lifetime"];
                (Field<glm::vec3>)dashEffectModel["ExplosionEffect"]["ExplosionOrigin"] = glm::vec3(0, 1, 0) + (0.2f * controller->Movement());



                
                auto animationChildren = dashEffectModel.ChildrenWithComponent("Animation");
                
                for (auto animationEntity : animationChildren) {
                    (Field<bool>)animationEntity["Animation"]["Play"] = false;
                }
                */
            }
            
        }
    }
    return true;
}
