#ifndef WeaponBehaviour_h__
#define WeaponBehaviour_h__

#include "Core/System.h"
#include "Rendering/IRenderer.h"
#include "Core/Octree.h"
#include "Collision/EntityAABB.h"

class WeaponBehaviour : public System
{
    friend class WeaponSystem;

public:
    WeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree, EntityWrapper player, EntityWrapper firstPersonWeapon, EntityWrapper thirdPersonWeapon)
        : System(systemParams)
        , m_Renderer(renderer)
        , m_CollisionOctree(collisionOctree)
        , m_Player(player)
        , m_FirstPerson(firstPersonWeapon)
        , m_ThirdPerson(thirdPersonWeapon)
    { }
    virtual ~WeaponBehaviour() = default;

    WeaponBehaviour(const WeaponBehaviour&) = delete;
    WeaponBehaviour& operator=(const WeaponBehaviour&) = delete;

    virtual void Fire() = 0;
    virtual void CeaseFire() { }
    virtual void Reload() { }
    virtual void Update(double dt) { }

protected:
    IRenderer* m_Renderer;
    Octree<EntityAABB>* m_CollisionOctree;
    EntityWrapper m_Player;
    EntityWrapper m_FirstPerson;
    EntityWrapper m_ThirdPerson;
};

#endif
