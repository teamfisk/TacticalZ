#include "Systems/Weapon/SidearmWeaponBehaviour.h"

void SidearmWeaponBehaviour::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt)
{
    double& cooldown = cWeapon["FireCooldown"];
    if (cooldown > 0) {
        cooldown -= dt;
        if (cooldown < 0) {
            cooldown = 0;
        }
    }
    WeaponBehaviour::UpdateComponent(entity, cWeapon, dt);
}

void SidearmWeaponBehaviour::UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt)
{
    CheckAmmo(cWeapon, wi);

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

        magAmmo = magSize;
        isReloading = false;
        if (wi.FirstPersonEntity.Valid()) {
            wi.FirstPersonEntity.FirstChildByName("ViewModel")["Model"]["Visible"] = true;
        }
        if (wi.ThirdPersonEntity.Valid()) {
            wi.ThirdPersonEntity["Model"]["Visible"] = true;
        }
    }
    double reloadTime = cWeapon["ReloadTime"];
    if (isReloading && reloadTimer <= reloadTime / 2) {

    }

    // Update first person run animation
    ComponentWrapper cPlayer = wi.Player["Player"];
    ComponentWrapper cPhysics = wi.Player["Physics"];
    const float& movementSpeed = cPlayer["MovementSpeed"];
    float speed = glm::length((const glm::vec3&)cPhysics["Velocity"]);
    float animationWeight = glm::min(speed, movementSpeed) / movementSpeed;
    EntityWrapper rootNode = wi.FirstPersonEntity;
    if (rootNode.Valid()) {
        EntityWrapper blend = rootNode.FirstChildByName("MovementBlendSidearm");
        if (blend.Valid()) {
            (double&)blend["Blend"]["Weight"] = animationWeight;
        }
    }

    if ((bool)cWeapon["Automatic"] && canFire(cWeapon)) {
        fireBullet(cWeapon, wi);
    }
}

void SidearmWeaponBehaviour::OnPrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["TriggerHeld"] = true;
    if (canFire(cWeapon)) {
        fireBullet(cWeapon, wi);
    }
}

void SidearmWeaponBehaviour::OnCeasePrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["TriggerHeld"] = false;
}


void SidearmWeaponBehaviour::OnReload(ComponentWrapper cWeapon, WeaponInfo& wi)
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

    double reloadTime = cWeapon["ReloadTime"];
    double& reloadTimer = cWeapon["ReloadTimer"];

    // Start reload
    reloadQueued = true;
    reloadTimer = reloadTime;

    // Play animation
    playAnimationAndReturn(wi.FirstPersonEntity, "ActionBlend", "Reload");
    // Third person anim
    Events::AutoAnimationBlend eReloadBlend;
    eReloadBlend.RootNode = wi.ThirdPersonPlayerModel;
    eReloadBlend.NodeName = "Reload";
    eReloadBlend.Restart = true;
    eReloadBlend.Start = true;
    m_EventBroker->Publish(eReloadBlend);

    // Spawn explosion effect
    if (wi.FirstPersonEntity.Valid()) {
        if (IsClient) {
            EntityWrapper reloadEffectSpawner = wi.FirstPersonEntity.FirstChildByName("ReloadSpawner");
            if (reloadEffectSpawner.Valid()) {
                reloadEffectSpawner.DeleteChildren();
                SpawnerSystem::Spawn(reloadEffectSpawner, reloadEffectSpawner);
            }
        }
        wi.FirstPersonEntity.FirstChildByName("ViewModel")["Model"]["Visible"] = false;
    }
    if (wi.ThirdPersonEntity.Valid()) {
        if (IsServer) {
            EntityWrapper reloadEffectSpawner = wi.ThirdPersonEntity.FirstChildByName("ReloadSpawner");
            if (reloadEffectSpawner.Valid()) {
                reloadEffectSpawner.DeleteChildren();
                SpawnerSystem::Spawn(reloadEffectSpawner, reloadEffectSpawner);
            }
        }
        wi.ThirdPersonEntity["Model"]["Visible"] = false;
    }

    // Sound
    Events::PlaySoundOnEntity e;
    e.EmitterID = wi.Player.ID;
    e.FilePath = "Audio/weapon/Assault/AssaultWeaponReload.wav";
    m_EventBroker->Publish(e);
}

void SidearmWeaponBehaviour::OnEquip(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["FireCooldown"] = (double)cWeapon["EquipTime"];

    if(wi.ThirdPersonPlayerModel.Valid()) {
        Events::AutoAnimationBlend b1;
        b1.RootNode = wi.ThirdPersonPlayerModel;
        b1.NodeName = "SidearmWeapon";
        b1.SingleLevelBlend = true;
        m_EventBroker->Publish(b1);
    }
}

void SidearmWeaponBehaviour::OnHolster(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    // Make sure the trigger is released if weapon is holstered while firing
    cWeapon["TriggerHeld"] = false;

    // Cancel any reload
    cWeapon["IsReloading"] = false;
    cWeapon["ReloadTimer"] = 0.0;
}

void SidearmWeaponBehaviour::fireBullet(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["FireCooldown"] = 60.0 / (double)cWeapon["RPM"];

    // Ammo
    int& magAmmo = cWeapon["MagazineAmmo"];
    if (magAmmo <= 0) {
        return;
    } else {
        magAmmo -= 1;
    }

    // Get weapon model based on current person
    EntityWrapper weaponModelEntity = getRelevantWeaponEntity(wi);
    if (!weaponModelEntity.Valid()) {
        return;
    }

    // Tracer
    EntityWrapper tracerSpawner = weaponModelEntity.FirstChildByName("WeaponMuzzle");
    if (tracerSpawner.Valid()) {
        glm::vec3 origin = Transform::AbsolutePosition(tracerSpawner);
        glm::vec3 direction = Transform::AbsoluteOrientation(tracerSpawner) * glm::vec3(0, 0, -1);
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
    playAnimationAndReturn(wi.FirstPersonEntity, "ActionBlend", "Fire");

    // Third person anim
    if (wi.ThirdPersonPlayerModel.Valid()) {
        Events::AutoAnimationBlend b1;
        b1.RootNode = wi.ThirdPersonPlayerModel;
        b1.NodeName = "Fire";
        b1.Restart = true;
        b1.Start = true;
        m_EventBroker->Publish(b1);
    }
}

bool SidearmWeaponBehaviour::canFire(ComponentWrapper cWeapon)
{
    bool triggerHeld = cWeapon["TriggerHeld"];
    bool cooldownPassed = (double)cWeapon["FireCooldown"] <= 0.0;
    bool isNotReloading = !(bool)cWeapon["IsReloading"];
    return triggerHeld && cooldownPassed && isNotReloading;
}

bool SidearmWeaponBehaviour::dealDamage(ComponentWrapper cWeapon, WeaponInfo& wi)
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

        bool gaveAmmo = false;

        if (victim.HasComponent("AssaultWeapon")) {
            int magazineSize = victim["AssaultWeapon"]["MagazineSize"];
            int magazineAmmo = victim["AssaultWeapon"]["MagazineAmmo"];

            int maxAmmo = victim["AssaultWeapon"]["MaxAmmo"];
            int ammo = victim["AssaultWeapon"]["Ammo"];

            if (magazineAmmo < magazineSize) {
                gaveAmmo = true;
            } else if (ammo < maxAmmo) {
                gaveAmmo = true;
            }
        } else if (victim.HasComponent("DefenderWeapon")) {
            int magazineSize = victim["DefenderWeapon"]["MagazineSize"];
            int magazineAmmo = victim["DefenderWeapon"]["MagazineAmmo"];

            int maxAmmo = victim["DefenderWeapon"]["MaxAmmo"];
            int ammo = victim["DefenderWeapon"]["Ammo"];

            if (magazineAmmo < magazineSize) {
                gaveAmmo = true;
            } else if (ammo < maxAmmo) {
                gaveAmmo = true;
            }
        }


        if (gaveAmmo) {
            EntityWrapper effectSpawner = wi.FirstPersonEntity.FirstChildByName("AmmoShareEffectSpawner");
            if (effectSpawner.Valid()) {
                if (effectSpawner.HasComponent("Spawner")) {
                    SpawnerSystem::Spawn(effectSpawner, effectSpawner);
                }
            }
        }
    }

    // Deal damage! 
    Events::PlayerDamage ePlayerDamage;
    ePlayerDamage.Inflictor = wi.Player;
    ePlayerDamage.Victim = victim;
    ePlayerDamage.Damage = damage;
    m_EventBroker->Publish(ePlayerDamage);

    return damage > 0;
}
/*

void SidearmWeaponBehaviour::giveAmmo(ComponentWrapper cWeapon, WeaponInfo& wi, EntityWrapper receiver)
{
    bool gaveAmmo = false;

    if (receiver.HasComponent("AssaultWeapon")) {
        int magazineSize = receiver["AssaultWeapon"]["MagazineSize"];
        int& magazineAmmo = receiver["AssaultWeapon"]["MagazineAmmo"];

        int maxAmmo = receiver["AssaultWeapon"]["MaxAmmo"]; 
        int& ammo = receiver["AssaultWeapon"]["Ammo"];

        if(magazineAmmo < magazineSize) {
            magazineAmmo += 1;
            gaveAmmo = true;
        } else if (ammo < maxAmmo) {
            ammo += 1;
            gaveAmmo = true;
        }
    } else if (receiver.HasComponent("DefenderWeapon")) {
        int magazineSize = receiver["DefenderWeapon"]["MagazineSize"];
        int& magazineAmmo = receiver["DefenderWeapon"]["MagazineAmmo"];

        int maxAmmo = receiver["DefenderWeapon"]["MaxAmmo"];
        int& ammo = receiver["DefenderWeapon"]["Ammo"];

        if (magazineAmmo < magazineSize) {
            magazineAmmo += 1;
            gaveAmmo = true;
        } else if (ammo < maxAmmo) {
            ammo += 1;
            gaveAmmo = true;
        }
    }


    if(gaveAmmo) {
        EntityWrapper effectSpawner = wi.FirstPersonEntity.FirstChildByName("AmmoShareEffectSpawner");
        if(effectSpawner.Valid()) {
            if (effectSpawner.HasComponent("Spawner")) {
                SpawnerSystem::Spawn(effectSpawner, effectSpawner);
            }
        }
    }
}*/

void SidearmWeaponBehaviour::CheckAmmo(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    // Only check ammo client side
    if (!IsClient) {
        return;
    }

    // Only handle ammo check for the local player
    if (wi.Player != LocalPlayer) {
        return;
    }

    // Make sure the player isn't checking from the grave
    if (!wi.Player.Valid()) {
        return;
    }

    // 3D-pick middle of screen
    Rectangle viewport = m_Renderer->GetViewportSize();
    glm::vec2 centerScreen(viewport.Width / 2, viewport.Height / 2);
    // TODO: Some horizontal spread
    PickData pickData = m_Renderer->Pick(centerScreen);
    EntityWrapper victim(m_World, pickData.Entity);
    if (!victim.Valid()) {
        RemoveFrindlyAmmoHUD(wi);
        return;
    }

    // Don't let us somehow shoot ourselves in the foot
    if (victim == LocalPlayer) {
        RemoveFrindlyAmmoHUD(wi);
        return;
    }

    // Only care about players being hit
    if (!victim.HasComponent("Player")) {
        victim = victim.FirstParentWithComponent("Player");
    }
    if (!victim.Valid()) {
        RemoveFrindlyAmmoHUD(wi);
        return;
    }


    // If friendly fire, reduce damage to 0 (needed to make Boosts, Ammosharing work)
    if ((ComponentInfo::EnumType)victim["Team"]["Team"] == (ComponentInfo::EnumType)wi.Player["Team"]["Team"]) {
        int magazineAmmo = 0;
        int ammo = 0;
        if (victim.HasComponent("AssaultWeapon")) {
            magazineAmmo = (int)victim["AssaultWeapon"]["MagazineAmmo"];
            ammo = (int)victim["AssaultWeapon"]["Ammo"];

        } else if (victim.HasComponent("DefenderWeapon")) {
            magazineAmmo = (int)victim["DefenderWeapon"]["MagazineAmmo"];
            ammo = (int)victim["DefenderWeapon"]["Ammo"];
        }


        EntityWrapper friendlyAmmoHudSpawner = wi.FirstPersonPlayerModel.FirstChildByName("FriendlyAmmoAttachment");
        if (friendlyAmmoHudSpawner.Valid()) {

            auto children = m_World->GetDirectChildren(friendlyAmmoHudSpawner.ID);

            if (children.first == children.second) {
                if (friendlyAmmoHudSpawner.HasComponent("Spawner")) {
                    EntityWrapper friendlyAmmoHud = SpawnerSystem::Spawn(friendlyAmmoHudSpawner, friendlyAmmoHudSpawner);
                    if (friendlyAmmoHud.Valid()) {
                        EntityWrapper textEntity = friendlyAmmoHud.FirstChildByName("MagazineAmmo");
                        if (textEntity.Valid()) {
                            if (textEntity.HasComponent("Text")) {
                                (std::string&)textEntity["Text"]["Content"] = std::to_string(magazineAmmo);
                            }
                        }

                        EntityWrapper ammoTextEntity = friendlyAmmoHud.FirstChildByName("Ammo");
                        if (ammoTextEntity.Valid()) {
                            if (ammoTextEntity.HasComponent("Text")) {
                                (std::string&)ammoTextEntity["Text"]["Content"] = std::to_string(ammo);
                            }
                        }
                    }
                }
            } else {
                EntityWrapper textEntity = friendlyAmmoHudSpawner.FirstChildByName("MagazineAmmo");
                if (textEntity.Valid()) {
                    if (textEntity.HasComponent("Text")) {
                        (std::string&)textEntity["Text"]["Content"] = std::to_string(magazineAmmo);
                    }
                }

                EntityWrapper ammoTextEntity = friendlyAmmoHudSpawner.FirstChildByName("Ammo");
                if (ammoTextEntity.Valid()) {
                    if (ammoTextEntity.HasComponent("Text")) {
                        (std::string&)ammoTextEntity["Text"]["Content"] = std::to_string(ammo);
                    }
                }
            }
        }
    }
}

void SidearmWeaponBehaviour::RemoveFrindlyAmmoHUD(WeaponInfo& wi)
{
    EntityWrapper friendlyAmmoHudSpawner = wi.FirstPersonPlayerModel.FirstChildByName("FriendlyAmmoAttachment");
    if (friendlyAmmoHudSpawner.Valid()) {
        friendlyAmmoHudSpawner.DeleteChildren();
    }
}
