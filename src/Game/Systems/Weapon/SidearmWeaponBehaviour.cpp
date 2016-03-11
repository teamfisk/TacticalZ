#include "Systems/Weapon/SidearmWeaponBehaviour.h"

void SidearmWeaponBehaviour::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt)
{
    Field<double> cooldown = cWeapon["FireCooldown"];
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

void SidearmWeaponBehaviour::OnEquip(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    cWeapon["FireCooldown"] = (double)cWeapon["EquipTime"];
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

    // Get weapon model based on current person
    EntityWrapper weaponModelEntity = getRelevantWeaponModelEntity(wi);
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
        ((Field<glm::vec3>)ray["Transform"]["Scale"]).z(distance / 100.f);
    }
}

bool SidearmWeaponBehaviour::canFire(ComponentWrapper cWeapon)
{
    bool triggerHeld = cWeapon["TriggerHeld"];
    Field<double> cooldown = cWeapon["FireCooldown"];
    // TODO: Ammo checks
    return triggerHeld && cooldown <= 0.0;
}