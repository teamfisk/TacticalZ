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
            e.EmitterID = wi.Player.ID;
            e.FilePath = "Audio/weapon/Zoom.wav";
            m_EventBroker->Publish(e);
        } else {
            isReloading = false;
            playAnimationAndReturn(wi.FirstPersonEntity, "FinalBlend", "ReloadEnd");
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
    Field<double> reloadTimer = cWeapon["ReloadTimer"];
   
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
                if (IsServer) {
                    SpawnerSystem::Spawn(attachment, attachment);
                }

                if (IsClient) {
                    EntityWrapper root = wi.FirstPersonEntity;
                    if (root.Valid()) {
                        EntityWrapper animationNode = root.FirstChildByName("Shield");
                        if (animationNode.Valid()) {
                            Events::AutoAnimationBlend eFireBlend;
                            eFireBlend.RootNode = root;
                            eFireBlend.NodeName = "Shield";
                            eFireBlend.Restart = true;
                            eFireBlend.Start = true;
                            m_EventBroker->Publish(eFireBlend);
                        }
                    }
                }
            } else {
                attachment.DeleteChildren();

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
                }
            }
        }
    }

    return false;
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

    int numPellets = cWeapon["NumPellets"];
    float spreadAngle = cWeapon["SpreadAngle"];
    std::uniform_real_distribution<float> randomSpreadAngle(-spreadAngle, spreadAngle);

    // Calculate pellet angles
    // HACK: Random for now?
    // TODO: Make distribution even for each quadrant
    std::vector<glm::vec2> pelletAngles;
    for (int i = 0; i < numPellets; i++) {
        pelletAngles.push_back(glm::vec2(randomSpreadAngle(m_RandomEngine), randomSpreadAngle(m_RandomEngine)));
    }

    double pelletDamage = (double)cWeapon["BaseDamage"] / numPellets;

    // View punch
    if (IsClient) {
        EntityWrapper camera = wi.Player.FirstChildByName("Camera");
        if (camera.Valid()) {
            Field<glm::vec3> cameraOrientation = camera["Transform"]["Orientation"];
            float viewPunch = cWeapon["ViewPunch"];
            float maxTravelAngle = cWeapon["MaxTravelAngle"];
            Field<float> currentTravel = cWeapon["CurrentTravel"];
            if (currentTravel < maxTravelAngle) {
                float change = viewPunch;
                if (currentTravel + change > maxTravelAngle) {
                    change = maxTravelAngle - currentTravel;
                }
                cameraOrientation.x(cameraOrientation.x() + change);
                currentTravel += change;
            }
        }
    }

    // Tracers
    EntityWrapper weaponModelEntity;
    if (wi.Player == LocalPlayer) {
        weaponModelEntity = wi.FirstPersonEntity;
    } else {
        weaponModelEntity = wi.ThirdPersonEntity;
    }
    if (weaponModelEntity.Valid()) {
        EntityWrapper spawner = weaponModelEntity.FirstChildByName("WeaponMuzzle");
        for (auto& angles : pelletAngles) {
            glm::vec3 direction = Transform::AbsoluteOrientation(spawner) * glm::quat(glm::vec3(angles, 0.f)) * glm::vec3(0, 0, -1);
            float distance = traceRayDistance(Transform::AbsolutePosition(spawner), direction);
            EntityWrapper ray = SpawnerSystem::Spawn(spawner);
            ((Field<glm::vec3>)ray["Transform"]["Scale"]).z(distance / 100.f);
            Field<glm::vec3> orientation = ray["Transform"]["Orientation"];
            orientation.x(orientation.x() + angles.x);
            orientation.y(orientation.y() + angles.y);
            glm::vec3 trajectory = direction * distance;
            dealDamage(cWeapon, wi, direction, pelletDamage);
        }
    }

    // Play animation
    playAnimationAndReturn(wi.FirstPersonEntity, "FinalBlend", "Fire");

    // Sound
    Events::PlaySoundOnEntity e;
    e.EmitterID = wi.Player.ID;
    e.FilePath = "Audio/weapon/Blast.wav";
    m_EventBroker->Publish(e);
}

void DefenderWeaponBehaviour::dealDamage(ComponentWrapper cWeapon, WeaponInfo& wi, glm::vec3 direction, double damage)
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
    
    glm::vec3 maxRange = direction * 2.f;
    EntityWrapper camera = wi.Player.FirstChildByName("Camera");
    glm::vec3 cameraPosition = Transform::AbsolutePosition(camera);
    if (!camera.Valid()) {
        return;
    }
    Rectangle screenResolution = m_Renderer->GetViewportSize();
    glm::vec2 centerScreen = glm::vec2(screenResolution.Width / 2, screenResolution.Height / 2);
    glm::vec2 screenCoords = cameraFromEntity(m_CurrentCamera).WorldToScreen(cameraPosition + maxRange, m_Renderer->GetViewportSize());
    PickData pickData = m_Renderer->Pick(centerScreen + screenCoords);
    EntityWrapper victim(m_World, pickData.Entity);
    if (!victim.Valid()) {
        return;
    }

    // Don't let us shoot ourselves in the foot somehow
    if (victim == LocalPlayer) {
        return;
    }

    // Only care about players being hit
    if (!victim.HasComponent("Player")) {
        victim = victim.FirstParentWithComponent("Player");
    }
    if (!victim.Valid()) {
        return;
    }

    // Check for friendly fire
    if ((ComponentInfo::EnumType)victim["Team"]["Team"] == (ComponentInfo::EnumType)wi.Player["Team"]["Team"]) {
        return;
    }

    // Deal damage! 
    Events::PlayerDamage ePlayerDamage;
    ePlayerDamage.Inflictor = wi.Player;
    ePlayerDamage.Victim = victim;
    ePlayerDamage.Damage = damage;
    m_EventBroker->Publish(ePlayerDamage);
    LOG_DEBUG("Damage: %f", damage);
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
