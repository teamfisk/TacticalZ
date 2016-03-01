#include "WeaponBehaviour.h"
#include "Collision/Collision.h"
#include "Core/EPlayerDamage.h"

class DefenderWeaponBehaviour : public WeaponBehaviour<DefenderWeaponBehaviour>
{
public:
    DefenderWeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree)
        : System(systemParams)
        , WeaponBehaviour(systemParams, "DefenderWeapon", renderer, collisionOctree)
        , m_RandomEngine(m_RandomDevice())
    { }

    void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt) override;
    void UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt) override;
    void OnPrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    void OnCeasePrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    bool OnInputCommand(ComponentWrapper cWeapon, WeaponInfo& wi, const Events::InputCommand& e) override;

private:
    std::random_device m_RandomDevice;
    std::mt19937 m_RandomEngine;

    // Weapon functions
    void fireShell(ComponentWrapper cWeapon, WeaponInfo& wi);
    void dealDamage(ComponentWrapper cWeapon, WeaponInfo& wi, glm::vec3 direction, double damage);

    // Utility
    Camera cameraFromEntity(EntityWrapper camera);
};