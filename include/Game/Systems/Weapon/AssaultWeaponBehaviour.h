#include "Sound/EPlaySoundOnEntity.h"
#include "Collision/Collision.h"
#include "Rendering/AnimationSystem.h"
#include "WeaponBehaviour.h"
#include "../SpawnerSystem.h"
#include "Core/EPlayerDamage.h"
#include "Core/EShoot.h"


class AssaultWeaponBehaviour : public WeaponBehaviour
{
public:
    AssaultWeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree, EntityWrapper weaponEntity);
    
    virtual void Fire() override;
    virtual void CeaseFire() override;
    virtual void Reload() override;

    virtual void Update(double dt) override;

private:
    EntityWrapper m_FirstPersonModel;
    // State
    bool m_Firing = false;
    bool m_Reloading = false;
    double m_ReloadTimer = 0.0;
    EntityWrapper m_ReloadImpersonator;
    double m_TimeSinceLastFire = 0.0;

    EventRelay<WeaponBehaviour, Events::AnimationComplete> m_EAnimationComplete;
    bool OnAnimationComplete(Events::AnimationComplete& e);

    bool hasAmmo();
    void fireRound();
    void spawnTracer();
    float traceRayDistance(glm::vec3 origin, glm::vec3 direction);
    void playSound();
    void viewPunch();
    void finishReload();
    void playShootAnimation();
    void playIdleAnimation();
    void playReloadAnimation();
    bool shoot(double damage);
    void showHitMarker();
};
