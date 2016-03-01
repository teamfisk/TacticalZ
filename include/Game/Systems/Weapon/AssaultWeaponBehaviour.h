#ifndef AssaultWeaponBehaviour_h__
#define AssaultWeaponBehaviour_h__

#include "Sound/EPlaySoundOnEntity.h"
#include "Collision/Collision.h"
#include "Core/ConfigFile.h"
#include "WeaponBehaviour.h"
#include "../SpawnerSystem.h"
#include "Core/EPlayerDamage.h"
#include "Core/EShoot.h"

class AssaultWeaponBehaviour : public WeaponBehaviour<AssaultWeaponBehaviour>
{
public:
    AssaultWeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree)
        : WeaponBehaviour(systemParams, "AssaultWeapon", renderer, collisionOctree) 
    { }

protected:
    virtual void OnPrimaryFire(WeaponInfo& wi) override;
    virtual void OnCeasePrimaryFire(WeaponInfo& wi) override;
    virtual void OnReload(WeaponInfo& wi) override;

private:
    // State
    bool m_Firing = false;
    bool m_Reloading = false;
    double m_ReloadTimer = 0.0;
    double m_TimeSinceLastFire = 0.0;
    EntityWrapper m_FirstPersonReloadImpostor;

    bool hasAmmo();
    void fireRound();
    void spawnTracer();
    float traceRayDistance(glm::vec3 origin, glm::vec3 direction);
    void playFireSound();
    void playEmptySound();
    void viewPunch();
    void finishReload();
    void playShootAnimation();
    void playIdleAnimation();
    void playReloadAnimation();
    bool shoot(double damage);
    void showHitMarker();
};

#endif