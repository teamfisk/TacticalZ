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
    if (canFire(cWeapon)) {
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

void SidearmWeaponBehaviour::OnHolster(ComponentWrapper cWeapon, WeaponInfo& wi)
{
    // Make sure the trigger is released if weapon is holstered while firing
    cWeapon["TriggerHeld"] = false;

    // Cancel any reload
    cWeapon["IsReloading"] = false;
    cWeapon["ReloadTimer"] = 0.0;
    
    LOG_DEBUG("HOLSTER");
}

void SidearmWeaponBehaviour::fireBullet(ComponentWrapper cWeapon, WeaponInfo& wi)
{

}

bool SidearmWeaponBehaviour::canFire(ComponentWrapper cWeapon)
{
    bool triggerHeld = cWeapon["TriggerHeld"];
    double& cooldown = cWeapon["FireCooldown"];
    // TODO: Ammo checks
    return triggerHeld && cooldown <= 0.0;
}