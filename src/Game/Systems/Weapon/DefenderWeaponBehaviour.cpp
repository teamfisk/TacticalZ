#include "Systems/Weapon/DefenderWeaponBehaviour.h"

void DefenderWeaponBehaviour::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt)
{
    Field<double> fireCooldown = cWeapon["FireCooldown"];
    fireCooldown = glm::max(0.0, fireCooldown - dt);

    WeaponBehaviour::UpdateComponent(entity, cWeapon, dt);
}

void DefenderWeaponBehaviour::UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt)
{
    // Decrement reload timer
    Field<double> reloadTimer = cWeapon["ReloadTimer"];
    reloadTimer = glm::max(0.0, reloadTimer - dt);

    // Start reloading automatically if at 0 mag ammo
    Field<int> magAmmo = cWeapon["MagazineAmmo"];
    if (m_ConfigAutoReload && magAmmo <= 0) {
        OnReload(cWeapon, wi);
    }

    // Handle reloading
    Field<bool> isReloading = cWeapon["IsReloading"];
    if (isReloading && reloadTimer <= 0.0) {
        double reloadTime = cWeapon["ReloadTime"];
        Field<int> magSize = cWeapon["MagazineSize"];
        Field<int> ammo = cWeapon["Ammo"];
        if (magAmmo < magSize && ammo > 0) {
            ammo -= 1;
            magAmmo += 1;
            reloadTimer = reloadTime;
            Events::PlaySoundOnEntity e;
            e.Emitter = wi.Player;
            e.FilePath = "Audio/weapon/Zoom.wav";
            m_EventBroker->Publish(e);
        } else {
            isReloading = false;
            playAnimationAndReturn(wi.FirstPersonEntity, "FinalBlend", "ReloadEnd");

            if (wi.ThirdPersonPlayerModel.Valid()) {
                Events::AutoAnimationBlend eFireIdle;
                eFireIdle.Duration = 0.2;
                eFireIdle.RootNode = wi.ThirdPersonPlayerModel;
                eFireIdle.NodeName = "Fire";
                eFireIdle.Start = false;
                m_EventBroker->Publish(eFireIdle);
            }

        }
    }

    // Restore view angle
    if (IsClient) {
        Field<float> currentTravel = cWeapon["CurrentTravel"];
        Field<float> returnSpeed = cWeapon["ViewReturnSpeed"];
        if (currentTravel > 0) {
            float change = returnSpeed * dt;
            currentTravel = glm::max(0.f, currentTravel - change);
            EntityWrapper camera = wi.Player.FirstChildByName("Camera");
            if (camera.Valid()) {
                Field<glm::vec3> cameraOrientation = camera["Transform"]["Orientation"];
                cameraOrientation.x(cameraOrientation.x() - change);
            }
        }
    }

    // Fire if we're able to fire
    if (canFire(cWeapon, wi)) {
        fireShell(cWeapon, wi);
    }
}

void DefenderWeaponBehaviour::OnPrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["TriggerHeld"] = true;
    if (canFire(cWeapon, wi)) {
        fireShell(cWeapon, wi);
    }
}

void DefenderWeaponBehaviour::OnCeasePrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["TriggerHeld"] = false;
}

void DefenderWeaponBehaviour::OnReload(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    Field<bool> isReloading = cWeapon["IsReloading"];
    if (isReloading) {
        return;
    }

    Field<int> magAmmo = cWeapon["MagazineAmmo"];
    Field<int> magSize = cWeapon["MagazineSize"];
    if (magAmmo >= magSize) {
        return;
    }
    Field<int> ammo = cWeapon["Ammo"];
    if (ammo <= 0) {
        return;
    }

    double reloadTime = cWeapon["ReloadTime"];
    Field<double>  reloadTimer = cWeapon["ReloadTimer"];
   
    // Start reload
    isReloading = true;
    reloadTimer = reloadTime;

    // Play animation
    if (wi.FirstPersonEntity.Valid()) {
        Events::AutoAnimationBlend eBlendStart;
        eBlendStart.RootNode = wi.FirstPersonEntity;
        eBlendStart.NodeName = "ReloadStart";
        eBlendStart.Restart = true;
        eBlendStart.Start = true;
        m_EventBroker->Publish(eBlendStart);
        Events::AutoAnimationBlend eBlendLoop;
        eBlendLoop.RootNode = wi.FirstPersonEntity;
        eBlendLoop.NodeName = "ReloadLoop";
        eBlendLoop.Restart = true;
        eBlendLoop.Start = true;
        eBlendLoop.AnimationEntity = wi.FirstPersonEntity.FirstChildByName("ReloadStart");
        m_EventBroker->Publish(eBlendLoop);
    }

    if(wi.ThirdPersonPlayerModel.Valid()) {
        Events::AutoAnimationBlend eReload;
        eReload.Duration = 0.2;
        eReload.RootNode = wi.ThirdPersonPlayerModel;
        eReload.NodeName = "Reload";
        eReload.Restart = true;
        eReload.Start = true;
        m_EventBroker->Publish(eReload);
    }
}

void DefenderWeaponBehaviour::OnHolster(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    // Make sure the trigger is released if weapon is holstered while firing
    cWeapon["TriggerHeld"] = false;

    // Cancel any reload
    cWeapon["IsReloading"] = false;
    cWeapon["ReloadTimer"] = 0.0;
}

bool DefenderWeaponBehaviour::OnInputCommand(ComponentWrapper cWeapon, WeaponInfo& wi, const Events::InputCommand& e)
{
    if (e.Command == "SpecialAbility") {
        EntityWrapper attachment = wi.Player.FirstChildByName("ShieldAttachment");
        if (attachment.Valid()) {
            if (e.Value > 0) {

                if(wi.Player.Valid()) {
                    if(wi.Player.HasComponent("ShieldAbility")) {
                        (Field<bool>)wi.Player["ShieldAbility"]["Active"] = true;
                    }
                }

                if (IsServer) {
                    SpawnerSystem::Spawn(attachment, attachment);
                }

                EntityWrapper backAttachment = attachment.FirstChildByName("Back");
                if (backAttachment.Valid()) {
                    Events::AutoAnimationBlend eDeployShieldAttachement;
                    eDeployShieldAttachement.RootNode = backAttachment;
                    eDeployShieldAttachement.NodeName = "Deploy";
                    eDeployShieldAttachement.Restart = true;
                    eDeployShieldAttachement.Start = true;
                    m_EventBroker->Publish(eDeployShieldAttachement);
                }
                EntityWrapper frontAttachment = attachment.FirstChildByName("Front");
                if (frontAttachment.Valid()) {
                    Events::AutoAnimationBlend eDeployShieldAttachement;
                    eDeployShieldAttachement.RootNode = frontAttachment;
                    eDeployShieldAttachement.NodeName = "Deploy";
                    eDeployShieldAttachement.Restart = true;
                    eDeployShieldAttachement.Start = true;
                    m_EventBroker->Publish(eDeployShieldAttachement);

                }

                if (IsClient) {
                    EntityWrapper root = wi.FirstPersonEntity;
                    if (root.Valid()) {
                        EntityWrapper animationNode = root.FirstChildByName("Shield");
                        if (animationNode.Valid()) {
                            Events::AutoAnimationBlend eShieldBlend;
                            eShieldBlend.RootNode = root;
                            eShieldBlend.NodeName = "Shield";
                            eShieldBlend.Restart = true;
                            eShieldBlend.Start = true;
                            m_EventBroker->Publish(eShieldBlend);
                        }
                    }
                } else { // server only
                    EntityWrapper root = wi.ThirdPersonPlayerModel;
                    if (root.Valid()) {
                        Events::AutoAnimationBlend eShieldActivateBlend;
                        eShieldActivateBlend.RootNode = root;
                        eShieldActivateBlend.NodeName = "ActivateShield";
                        eShieldActivateBlend.Restart = true;
                        eShieldActivateBlend.Start = true;
                        m_EventBroker->Publish(eShieldActivateBlend);

                        EntityWrapper animationNode = root.FirstChildByName("ActivateShield");
                        if (animationNode.Valid()) {
                            Events::AutoAnimationBlend eShieldIdleBlend;
                            eShieldIdleBlend.RootNode = root;
                            eShieldIdleBlend.NodeName = "ShieldFront";
                            eShieldIdleBlend.Restart = true;
                            eShieldIdleBlend.Start = true;
                            eShieldIdleBlend.AnimationEntity = animationNode;
                            m_EventBroker->Publish(eShieldIdleBlend);
                        }
                    }
                }
            } else {
                attachment.DeleteChildren();

                if (wi.Player.Valid()) {
                    if (wi.Player.HasComponent("ShieldAbility")) {
                        (Field<bool>)wi.Player["ShieldAbility"]["Active"] = false;
                    }
                }

                if (IsClient) {
                    EntityWrapper root = wi.FirstPersonEntity;
                    if (root.Valid()) {
                        EntityWrapper animationNode = root.FirstChildByName("ActionBlend");
                        if (animationNode.Valid()) {
                            Events::AutoAnimationBlend eFireBlend;
                            eFireBlend.RootNode = root;
                            eFireBlend.NodeName = "ActionBlend";
                            eFireBlend.Restart = true;
                            eFireBlend.Start = true;
                            m_EventBroker->Publish(eFireBlend);
                        }
                    }
                } else { // server only
                    EntityWrapper root = wi.ThirdPersonPlayerModel;
                    if (root.Valid()) {
                        Events::AutoAnimationBlend eShieldDeactivateBlend;
                        eShieldDeactivateBlend.RootNode = root;
                        eShieldDeactivateBlend.NodeName = "ActivateShield";
                        eShieldDeactivateBlend.Restart = true;
                        eShieldDeactivateBlend.Start = true;
                        eShieldDeactivateBlend.Reverse = true;
                        m_EventBroker->Publish(eShieldDeactivateBlend);

                        EntityWrapper animationNode = root.FirstChildByName("ActivateShield");
                        if (animationNode.Valid()) {
                            Events::AutoAnimationBlend eActionBlend;
                            eActionBlend.RootNode = root;
                            eActionBlend.NodeName = "ActionBlend";
                            eActionBlend.AnimationEntity = animationNode;
                            m_EventBroker->Publish(eActionBlend);
                        }
                    }
                }
            }
        }
    }

    return false;
}


void DefenderWeaponBehaviour::OnEquip(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    if (wi.ThirdPersonPlayerModel.Valid()) {
        Events::AutoAnimationBlend b1;
        b1.RootNode = wi.ThirdPersonPlayerModel;
        b1.NodeName = "DefenderWeapon";
        b1.SingleLevelBlend = true;
        m_EventBroker->Publish(b1);
    }
}

void DefenderWeaponBehaviour::fireShell(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["FireCooldown"] = 60.0 / (double)cWeapon["RPM"];

    // Stop reloading
    Field<bool> isReloading = cWeapon["IsReloading"];
    isReloading = false;

    // Ammo
    Field<int> magAmmo = cWeapon["MagazineAmmo"];
    if (magAmmo <= 0) {
        return;
    } else {
        magAmmo -= 1;
    }

    // We can't really do any valuable calculations without a valid camera
    if (!m_CurrentCamera.Valid()) {
        return;
    }

    int numPellets = cWeapon["NumPellets"];

    // Create a spread pattern
    std::vector<glm::vec2> pattern;
    // The first pellet is always centered
    pattern.push_back(glm::vec2(0, 0));
    // Any additional pellets form circles around the middle
    int numOuterPellets = numPellets - 1;
    float angleIncrement = glm::two_pi<float>() / numOuterPellets;
    for (int i = 0; i < numOuterPellets; ++i) {
        float angle = angleIncrement * i;
        glm::vec2 pellet = glm::vec2(glm::cos(angle), glm::sin(angle));
        pattern.push_back(pellet);
    }

    // Deal damage (clientside)
    dealDamage(cWeapon, wi, pattern);

    // Spawn tracers
    spawnTracers(cWeapon, wi, pattern);

    // View punch
    //if (IsClient) {
    //    EntityWrapper camera = wi.Player.FirstChildByName("Camera");
    //    if (camera.Valid()) {
    //        glm::vec3& cameraOrientation = camera["Transform"]["Orientation"];
    //        float viewPunch = cWeapon["ViewPunch"];
    //        float maxTravelAngle = cWeapon["MaxTravelAngle"];
    //        float& currentTravel = cWeapon["CurrentTravel"];
    //        if (currentTravel < maxTravelAngle) {
    //            float change = viewPunch;
    //            if (currentTravel + change > maxTravelAngle) {
    //                change = maxTravelAngle - currentTravel;
    //            }
    //            cameraOrientation.x += change;
    //            currentTravel += change;
    //        }
    //    }
    //}

    // Play animation
    playAnimationAndReturn(wi.FirstPersonEntity, "FinalBlend", "Fire");
    playAnimationAndReturn(wi.ThirdPersonPlayerModel, "FinalBlend", "Fire");

    // Sound
    Events::PlaySoundOnEntity e;
    e.Emitter = wi.Player;
    e.FilePath = "Audio/weapon/Blast.wav";
    m_EventBroker->Publish(e);
}

void DefenderWeaponBehaviour::spawnTracers(ComponentWrapper cWeapon, WeaponInfo& wi, std::vector<glm::vec2> pattern)
{
    EntityWrapper muzzle = getRelevantWeaponEntity(wi).FirstChildByName("WeaponMuzzle");
    if (!muzzle.Valid()) {
        return;
    }

    EntityWrapper camera = wi.Player.FirstChildByName("Camera");
    if (!camera.Valid()) {
        return;
    }

    float spreadAngle = glm::radians((float)cWeapon["SpreadAngle"]);

    for (auto& pellet : pattern) {
        glm::quat pelletRotation = glm::quat(TransformSystem::AbsoluteOrientationEuler(camera)) * glm::quat(glm::vec3(pellet.y, pellet.x, 0) * spreadAngle);
        glm::vec3 cameraPosition = TransformSystem::AbsolutePosition(camera);
        glm::vec3 direction = pelletRotation * glm::vec3(0, 0, -1);

        float distance;
        glm::vec3 hitPosition;
        if (Collision::EntityFirstHitByRay(Ray(cameraPosition, direction), m_CollisionOctree, distance, hitPosition) != boost::none) { // don't spawn ray if you "miss the world"

            EntityWrapper ray = SpawnerSystem::Spawn(muzzle);
            if (ray.Valid()) {
                ComponentWrapper cTransform = ray["Transform"];
                Field<glm::vec3> rayOrigin = cTransform["Position"];
                Field<glm::vec3> rayOrientation = cTransform["Orientation"];
                Field<glm::vec3> rayScale = cTransform["Scale"];

                glm::vec3 muzzlePosition = TransformSystem::AbsolutePosition(muzzle);
                glm::quat muzzleOrientation = TransformSystem::AbsoluteOrientation(muzzle);

                rayOrigin = muzzlePosition;

                glm::vec3 muzzleToHit = hitPosition - muzzlePosition;
                glm::vec3 lookVector = glm::normalize(-muzzleToHit);
                float pitch = std::asin(-lookVector.y);
                float yaw = std::atan2(lookVector.x, lookVector.z);
                glm::quat orientation = glm::quat(glm::vec3(pitch, yaw, 0));
                rayOrientation = glm::eulerAngles(orientation);
                rayScale.x(glm::length(muzzleToHit));
            }
        }
    }
}

void DefenderWeaponBehaviour::dealDamage(ComponentWrapper cWeapon, WeaponInfo& wi, const std::vector<glm::vec2>& pattern)
{
    // Only deal damage client side
    if (!IsClient) {
        return;
    }

    // Only handle shooting for the local player
    if (wi.Player != LocalPlayer) {
        return;
    }

    // Make sure the player isn't shooting from the grave
    if (!wi.Player.Valid()) {
        return;
    }

    // Convert the spread angle to screen coordinates, taking FOV into account
    Rectangle res = m_Renderer->GetViewportSize();
    float spreadAngle = glm::radians((float)cWeapon["SpreadAngle"]);
    float nearClip = (double)m_CurrentCamera["Camera"]["NearClip"];
    float farClip = (double)m_CurrentCamera["Camera"]["FarClip"];
    float yFOV = glm::radians((double)m_CurrentCamera["Camera"]["FOV"]);
    //float yRefFOV = glm::radians(59.f);
    //float yRef = glm::tan(yRefFOV) * nearClip;
    //float yRatio = yRef / (glm::tan(yFOV) * nearClip);
    //float yMax = (yRatio / 2.f) * spreadAngle * (res.Height / 2.f);
    float yRefFOV = glm::radians(59.f);
    float yRef = glm::tan(yRefFOV) * nearClip;
    float yRatio = yRef / (glm::tan(yFOV) * nearClip);
    float yMax = yRatio * (glm::tan(spreadAngle) * (farClip - nearClip)) * glm::pi<float>(); // ????? Good enough????

    LOG_DEBUG("Ratio: %f", yRatio);
    LOG_DEBUG("fatClip: %f", farClip);
    LOG_DEBUG("yMax: %f", yMax);
    double pelletDamage = (double)cWeapon["BaseDamage"] / pattern.size();

    // Pick!
    std::unordered_map<EntityWrapper, double> damageSum;
    glm::vec2 screenCenter(res.Width / 2.f, res.Height / 2.f);
    for (auto& pellet : pattern) {
        glm::vec2 pickCoord = screenCenter + (pellet * glm::vec2(yMax, yMax));
        PickData pick = m_Renderer->Pick(pickCoord);        

        EntityWrapper victim(m_World, pick.Entity);
        if (!victim.Valid()) {
            continue;
        }

        // Temp hit decal
        EntityWrapper hit = ResourceManager::Load<EntityFile>("Schema/Entities/HitTest.xml")->MergeInto(m_World);
        (Field<glm::vec3>)hit["Transform"]["Position"] = pick.Position;

        // Don't let us shoot ourselves in the foot somehow
        if (victim == LocalPlayer) {
            continue;
        }

        // Only care about players being hit
        if (!victim.HasComponent("Player")) {
            victim = victim.FirstParentWithComponent("Player");
            if (!victim.Valid()) {
                continue;
            }
        }

        // Check for friendly fire
        if ((ComponentInfo::EnumType)victim["Team"]["Team"] == (ComponentInfo::EnumType)wi.Player["Team"]["Team"]) {
            // TODO: Ammo sharing
            continue;
        }

        damageSum[victim] += pelletDamage;
        ((Field<glm::vec4>)hit["Model"]["Color"]).z(1.f);
    }

    // Deal damage!
    for (auto& kv : damageSum) {
        // Deal damage! 
        Events::PlayerDamage ePlayerDamage;
        ePlayerDamage.Inflictor = wi.Player;
        ePlayerDamage.Victim = kv.first;
        ePlayerDamage.Damage = kv.second;
        m_EventBroker->Publish(ePlayerDamage);
        LOG_DEBUG("Dealt %f damage to #%i", kv.second, kv.first.ID);
    }
}

bool DefenderWeaponBehaviour::canFire(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    bool triggerHeld = cWeapon["TriggerHeld"];
    bool cooldownPassed = (double)cWeapon["FireCooldown"] <= 0.0;
    bool isNotShielding = wi.Player.ChildrenWithComponent("Shield").size() == 0;
    return triggerHeld && cooldownPassed && isNotShielding;
}

Camera DefenderWeaponBehaviour::cameraFromEntity(EntityWrapper camera)
{
    ComponentWrapper cTransform = camera["Transform"];
    ComponentWrapper cCamera = camera["Camera"];
    Camera cam(
        (float)m_Renderer->Resolution().Width / m_Renderer->Resolution().Height, 
        (double)cCamera["FOV"], 
        (double)cCamera["NearClip"], 
        (double)cCamera["FarClip"]
    );
    cam.SetPosition(cTransform["Position"]);
    cam.SetOrientation(glm::quat((const glm::vec3&)cTransform["Orientation"]));
    return cam;
}
