#include "WeaponBehaviour.h"

class DefenderWeaponBehaviour : public WeaponBehaviour
{
public:
    DefenderWeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree, EntityWrapper weaponEntity);

    virtual void Fire() override;
    //virtual void CeaseFire() override;
    //virtual void Reload() override;

    //virtual void Update(double dt) override;

private:
    double m_TimeSinceLastFire = 0.0;
};