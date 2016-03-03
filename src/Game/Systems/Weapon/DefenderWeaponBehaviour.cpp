#include "Systems/Weapon/DefenderWeaponBehaviour.h"

void DefenderWeaponBehaviour::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt)
{
    double& fireCooldown = cWeapon["FireCooldown"];
    fireCooldown = glm::max(0.0, fireCooldown - dt);

    WeaponBehaviour::UpdateComponent(entity, cWeapon, dt);
}

void DefenderWeaponBehaviour::UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt)
{
    double& reloadTimer = cWeapon["ReloadTimer"];
    reloadTimer = glm::max(0.0, reloadTimer - dt);

    double reloadTime = cWeapon["ReloadTime"];
    bool& isReloading = cWeapon["IsReloading"];
    if (isReloading && reloadTimer <= 0.0) {
        int& magAmmo = cWeapon["MagazineAmmo"];
        int& magSize = cWeapon["MagazineSize"];
        int& ammo = cWeapon["Ammo"];
        if (magAmmo < magSize && ammo > 0) {
            ammo -= 1;
            magAmmo += 1;
            reloadTimer = reloadTime;
        } else {
            isReloading = false;
        }
    }

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
    bool& isReloading = cWeapon["IsReloading"];
    if (isReloading) {
        return;
    }

    int& magAmmo = cWeapon["MagazineAmmo"];
    int& magSize = cWeapon["MagazineSize"];
    if (magAmmo >= magSize) {
        return;
    }
    int& ammo = cWeapon["Ammo"];
    if (ammo <= 0) {
        return;
    }

    double reloadTime = cWeapon["ReloadTime"];
    double& reloadTimer = cWeapon["ReloadTimer"];
   
    // Start reload
    isReloading = true;
    reloadTimer = reloadTime;
}

bool DefenderWeaponBehaviour::OnInputCommand(ComponentWrapper cWeapon, WeaponInfo& wi, const Events::InputCommand& e)
{
    if (e.Command == "SpecialAbility" && IsServer) {
        EntityWrapper attachment = wi.Player.FirstChildByName("ShieldAttachment");
        if (attachment.Valid()) {
            if (e.Value > 0) {
                SpawnerSystem::Spawn(attachment, attachment);
            } else {
                attachment.DeleteChildren();
            }
        }
    }

    return false;
}

void DefenderWeaponBehaviour::fireShell(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["FireCooldown"] = 60.0 / (double)cWeapon["RPM"];

    // Stop reloading
    bool& isReloading = cWeapon["IsReloading"];
    isReloading = false;

    // Ammo
    int& magAmmo = cWeapon["MagazineAmmo"];
    if (magAmmo <= 0) {
        OnReload(cWeapon, wi);
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
        LOG_DEBUG("%f %f", pelletAngles[i].x, pelletAngles[i].y);
    }

    double pelletDamage = (double)cWeapon["BaseDamage"] / numPellets;

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
            ((glm::vec3&)ray["Transform"]["Scale"]).z = (distance / 100.f);
            glm::vec3& orientation = ray["Transform"]["Orientation"];
            orientation.x += angles.x;
            orientation.y += angles.y;
            glm::vec3 trajectory = direction * distance;
            dealDamage(cWeapon, wi, direction, pelletDamage);
        }
    }
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
    // TODO: Ammo checks
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
