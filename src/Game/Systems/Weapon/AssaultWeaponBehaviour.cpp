#include "Systems/Weapon/AssaultWeaponBehaviour.h"

void AssaultWeaponBehaviour::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt)
{
    double& fireCooldown = cWeapon["FireCooldown"];
    fireCooldown = glm::max(0.0, fireCooldown - dt);

    WeaponBehaviour::UpdateComponent(entity, cWeapon, dt);
}

void AssaultWeaponBehaviour::UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt)
{
    // Start reloading automatically if at 0 mag ammo
    int& magAmmo = cWeapon["MagazineAmmo"];
    if (m_ConfigAutoReload && magAmmo <= 0) {
        OnReload(cWeapon, wi);
    }

    // Only start reloading once we're done firing
    bool& reloadQueued = cWeapon["ReloadQueued"];
    double& fireCooldown = cWeapon["FireCooldown"];
    bool& isReloading = cWeapon["IsReloading"];
    if (reloadQueued && fireCooldown <= 0) {
        reloadQueued = fireCooldown;
        isReloading = true;
    }

    // Decrement reload timer
    double& reloadTimer = cWeapon["ReloadTimer"];
    if (isReloading) {
        reloadTimer = glm::max(0.0, reloadTimer - dt);
    }

    // Handle reloading
    if (isReloading && reloadTimer <= 0.0) {
        int& magSize = cWeapon["MagazineSize"];
        int& ammo = cWeapon["Ammo"];

        ammo = glm::max(0, ammo - (magSize - magAmmo));
        magAmmo = glm::min(magSize, ammo);
        isReloading = false;
    }    
    
    // Restore view angle
    if (IsClient) {
        float& currentTravel = cWeapon["CurrentTravel"];
        float& returnSpeed = cWeapon["ViewReturnSpeed"];
        if (currentTravel > 0) {
            float change = returnSpeed * dt;
            currentTravel = glm::max(0.f, currentTravel - change);
            EntityWrapper camera = wi.Player.FirstChildByName("Camera");
            if (camera.Valid()) {
                glm::vec3& cameraOrientation = camera["Transform"]["Orientation"];
                cameraOrientation.x -= change;
            }
        }
    }

    // Fire if we're able to fire
    if (canFire(cWeapon, wi)) {
        fireBullet(cWeapon, wi);
    }
}

void AssaultWeaponBehaviour::OnPrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["TriggerHeld"] = true;
    if (canFire(cWeapon, wi)) {
        fireBullet(cWeapon, wi);
    }
}

void AssaultWeaponBehaviour::OnCeasePrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["TriggerHeld"] = false;
}

void AssaultWeaponBehaviour::OnReload(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    bool& reloadQueued = cWeapon["ReloadQueued"];
    bool& isReloading = cWeapon["IsReloading"];
    if (reloadQueued || isReloading) {
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
    reloadQueued = true;
    reloadTimer = reloadTime;

    // Play animation
    EntityWrapper modelEntity = wi.FirstPersonEntity.Parent().Parent();
    if (modelEntity.Valid()) {
        EntityWrapper blendTree = modelEntity.FirstChildByName("PrimaryBlendTree");
        EntityWrapper reloadBlend = blendTree.FirstChildByName("Reload");

        Events::AutoAnimationBlend eFireBlend;
        eFireBlend.RootNode = modelEntity;
        eFireBlend.NodeName = "Reload";
        eFireBlend.Restart = true;
        eFireBlend.Start = true;
        m_EventBroker->Publish(eFireBlend);

        Events::AutoAnimationBlend eIdleBlend;
        eIdleBlend.RootNode = modelEntity;
        eIdleBlend.NodeName = "Idle";
        eIdleBlend.AnimationEntity = reloadBlend;
        eIdleBlend.Delay = -0.2;
        eIdleBlend.Duration = 0.2;
        m_EventBroker->Publish(eIdleBlend);
    }
}

void AssaultWeaponBehaviour::OnEquip(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["FireCooldown"] = (double)cWeapon["EquipTime"];
}

void AssaultWeaponBehaviour::OnHolster(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    // Make sure the trigger is released if weapon is holstered while firing
    cWeapon["TriggerHeld"] = false;

    // Cancel any reload
    cWeapon["IsReloading"] = false;
    cWeapon["ReloadTimer"] = 0.0;
}

void AssaultWeaponBehaviour::fireBullet(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["FireCooldown"] = 60.0 / (double)cWeapon["RPM"];

    // Ammo
    int& magAmmo = cWeapon["MagazineAmmo"];
    if (magAmmo <= 0) {
        return;
    } else {
        magAmmo -= 1;
    }

    // View punch
    if (IsClient) {
        EntityWrapper camera = wi.Player.FirstChildByName("Camera");
        if (camera.Valid()) {
            glm::vec3& cameraOrientation = camera["Transform"]["Orientation"];
            float viewPunch = cWeapon["ViewPunch"];
            float maxTravelAngle = cWeapon["MaxTravelAngle"];
            float& currentTravel = cWeapon["CurrentTravel"];
            if (currentTravel < maxTravelAngle) {
                float change = viewPunch;
                if (currentTravel + change > maxTravelAngle) {
                    change = maxTravelAngle - currentTravel;
                }
                cameraOrientation.x += change;
                currentTravel += change;
            }
        }
    }

    // Get weapon model based on current person
    EntityWrapper weaponModelEntity = getRelevantWeaponModelEntity(wi);
    if (!weaponModelEntity.Valid()) {
        return;
    }

    // Tracer
    EntityWrapper tracerSpawner = weaponModelEntity.FirstChildByName("WeaponMuzzle");
    if (tracerSpawner.Valid()) {
        glm::vec3 origin = Transform::AbsolutePosition(tracerSpawner);
        glm::vec3 direction = glm::quat(Transform::AbsoluteOrientationEuler(tracerSpawner)) * glm::vec3(0, 0, -1);
        float distance = traceRayDistance(origin, direction);
        EntityWrapper ray = SpawnerSystem::Spawn(tracerSpawner);
        if (ray.Valid()) {
            ((glm::vec3&)ray["Transform"]["Scale"]).z = distance;
        }
    }

    // Deal damage
    if (dealDamage(cWeapon, wi)) {
        // Show hit marker
        EntityWrapper hitMarkerSpawner = wi.Player.FirstChildByName("HitMarkerSpawner");
        if (hitMarkerSpawner.Valid()) {
            SpawnerSystem::Spawn(hitMarkerSpawner, hitMarkerSpawner);
            Events::PlaySoundOnEntity e;
            e.EmitterID = wi.Player.ID;
            e.FilePath = "Audio/weapon/hitclick.wav";
            m_EventBroker->Publish(e);
        }
    }
    
    // Play animation
    EntityWrapper modelEntity = wi.FirstPersonEntity.Parent().Parent();
    if (modelEntity.Valid()) {
        EntityWrapper blendTree = modelEntity.FirstChildByName("PrimaryBlendTree");
        EntityWrapper fireBlend = blendTree.FirstChildByName("Fire");

        Events::AutoAnimationBlend eFireBlend;
        eFireBlend.RootNode = modelEntity;
        eFireBlend.NodeName = "Fire";
        eFireBlend.Restart = true;
        eFireBlend.Start = true;
        eFireBlend.Duration = 0.0001;
        m_EventBroker->Publish(eFireBlend);

        Events::AutoAnimationBlend eIdleBlend;
        eIdleBlend.RootNode = modelEntity;
        eIdleBlend.NodeName = "Idle";
        eIdleBlend.AnimationEntity = fireBlend;
        eIdleBlend.Delay = -0.2;
        eIdleBlend.Duration = 0.2;
        m_EventBroker->Publish(eIdleBlend);
    }
}

bool AssaultWeaponBehaviour::canFire(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    bool triggerHeld = cWeapon["TriggerHeld"];
    bool cooldownPassed = (double)cWeapon["FireCooldown"] <= 0.0;
    bool isNotReloading = !(bool)cWeapon["IsReloading"];
    return triggerHeld && cooldownPassed && isNotReloading;
}

bool AssaultWeaponBehaviour::dealDamage(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    // Only deal damage client side
    if (!IsClient) {
        return false;
    }

    // Only handle damage for the local player
    if (wi.Player != LocalPlayer) {
        return false;
    }

    // Make sure the player isn't shooting from the grave
    if (!wi.Player.Valid()) {
        return false;
    }

    // 3D-pick middle of screen
    Rectangle viewport = m_Renderer->GetViewportSize();
    glm::vec2 centerScreen(viewport.Width / 2, viewport.Height / 2);
    // TODO: Some horizontal spread
    PickData pickData = m_Renderer->Pick(centerScreen);
    EntityWrapper victim(m_World, pickData.Entity);
    if (!victim.Valid()) {
        return false;
    }

    // Don't let us somehow shoot ourselves in the foot
    if (victim == LocalPlayer) {
        return false;
    }

    // Only care about players being hit
    if (!victim.HasComponent("Player")) {
        victim = victim.FirstParentWithComponent("Player");
    }
    if (!victim.Valid()) {
        return false;
    }

    double damage = cWeapon["BaseDamage"];
    // If friendly fire, reduce damage to 0 (needed to make Boosts, Ammosharing work)
    if ((ComponentInfo::EnumType)victim["Team"]["Team"] == (ComponentInfo::EnumType)wi.Player["Team"]["Team"]) {
        damage = 0;
    }

    // Deal damage! 
    Events::PlayerDamage ePlayerDamage;
    ePlayerDamage.Inflictor = wi.Player;
    ePlayerDamage.Victim = victim;
    ePlayerDamage.Damage = damage;
    m_EventBroker->Publish(ePlayerDamage);

    return damage > 0;
}
