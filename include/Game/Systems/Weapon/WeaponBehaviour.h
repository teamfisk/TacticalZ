#ifndef WeaponBehaviour_h__
#define WeaponBehaviour_h__

#include "Core/System.h"
#include "Core/Octree.h"
#include "Collision/EntityAABB.h"

class WeaponBehaviour : public System
{
public:
    WeaponBehaviour(SystemParams systemParams, Octree<EntityAABB>* collisionOctree, EntityWrapper weaponEntity)
        : System(systemParams)
        , m_CollisionOctree(collisionOctree)
        , m_Entity(weaponEntity)
    { }
    virtual ~WeaponBehaviour() = default;

    WeaponBehaviour(const WeaponBehaviour&) = delete;
    WeaponBehaviour& operator=(const WeaponBehaviour &) = delete;

    virtual void Fire() = 0;
    virtual void CeaseFire() { }
    virtual void Reload() { }
    virtual void Update(double dt) { }

protected:
    Octree<EntityAABB>* m_CollisionOctree;
    EntityWrapper m_Entity;
};

#endif
