#include "Systems/Weapon/DefenderWeaponBehaviour.h"

DefenderWeaponBehaviour::DefenderWeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree, EntityWrapper weaponEntity)
    : WeaponBehaviour(systemParams, renderer, collisionOctree, weaponEntity) { }

void DefenderWeaponBehaviour::Fire()
{

}

