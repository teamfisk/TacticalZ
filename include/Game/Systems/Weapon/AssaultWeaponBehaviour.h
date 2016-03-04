#ifndef AssaultWeaponBehaviour_h__
#define AssaultWeaponBehaviour_h__

#include "WeaponBehaviour.h"
#include "Collision/Collision.h"
#include "Core/EPlayerDamage.h"
#include "Sound/EPlaySoundOnEntity.h"
#include "Rendering/EAutoAnimationBlend.h"

class AssaultWeaponBehaviour : public WeaponBehaviour<AssaultWeaponBehaviour>
{
public:
    AssaultWeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree)
        : System(systemParams)
        , WeaponBehaviour(systemParams, "AssaultWeapon", renderer, collisionOctree)
        , m_RandomEngine(m_RandomDevice())
    { }

    void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt) override;
    void UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt) override;
    void OnPrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    void OnCeasePrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    void OnReload(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    void OnEquip(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    void OnHolster(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    //bool OnInputCommand(ComponentWrapper cWeapon, WeaponInfo& wi, const Events::InputCommand& e) override;

private:
    std::random_device m_RandomDevice;
    std::mt19937 m_RandomEngine;

    // Weapon functions
    void fireBullet(ComponentWrapper cWeapon, WeaponInfo& wi);
    //void dealDamage(ComponentWrapper cWeapon, WeaponInfo& wi, glm::vec3 direction, double damage);
    bool canFire(ComponentWrapper cWeapon, WeaponInfo& wi);
    bool dealDamage(ComponentWrapper cWeapon, WeaponInfo& wi);

    // Utility
    //Camera cameraFromEntity(EntityWrapper camera);
};

#endif