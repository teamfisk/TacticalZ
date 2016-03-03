#ifndef SidearmWeaponBehaviour_h__
#define SidearmWeaponBehaviour_h__

#include "WeaponBehaviour.h"
#include "Collision/Collision.h"
#include "Core/EPlayerDamage.h"

class SidearmWeaponBehaviour : public WeaponBehaviour<SidearmWeaponBehaviour>
{
public:
    SidearmWeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree)
        : System(systemParams)
        , WeaponBehaviour(systemParams, "SidearmWeapon", renderer, collisionOctree)
        , m_RandomEngine(m_RandomDevice())
    { }

    void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt) override;
    void UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt) override;
    void OnPrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    void OnCeasePrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    void OnEquip(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    void OnHolster(ComponentWrapper cWeapon, WeaponInfo& wi) override;

private:
    std::random_device m_RandomDevice;
    std::mt19937 m_RandomEngine;

    // Weapon functions
    void fireBullet(ComponentWrapper cWeapon, WeaponInfo& wi);
    //void dealDamage(WeaponInfo& wi, glm::vec3 direction, double damage);

    // Utility
    bool canFire(ComponentWrapper cWeapon);
    bool playerInFirstPerson(EntityWrapper player);
    //float traceRayDistance(glm::vec3 origin, glm::vec3 direction);
};

#endif