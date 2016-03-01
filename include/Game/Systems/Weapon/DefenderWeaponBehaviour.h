#include "WeaponBehaviour.h"
#include "Collision/Collision.h"
#include "Core/EPlayerDamage.h"
#include "Rendering/ESetCamera.h"

class DefenderWeaponBehaviour : public WeaponBehaviour<DefenderWeaponBehaviour>
{
public:
    DefenderWeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree)
        : System(systemParams)
        , WeaponBehaviour(systemParams, "DefenderWeapon", renderer, collisionOctree)
        , m_RandomEngine(m_RandomDevice())
    {
        EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &DefenderWeaponBehaviour::OnSetCamera);
    }

    void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt) override;
    void UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt) override;
    void OnPrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    void OnCeasePrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) override;
    bool OnInputCommand(ComponentWrapper cWeapon, WeaponInfo& wi, const Events::InputCommand& e) override;

private:
    std::random_device m_RandomDevice;
    std::mt19937 m_RandomEngine;
    EntityWrapper m_CurrentCamera;

    EventRelay<DefenderWeaponBehaviour, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(const Events::SetCamera& e);

    // Weapon functions
    void fireShell(ComponentWrapper cWeapon, WeaponInfo& wi);
    void dealDamage(ComponentWrapper cWeapon, WeaponInfo& wi, glm::vec3 direction, double damage);

    // Utility
    float traceRayDistance(glm::vec3 origin, glm::vec3 direction);
    Camera cameraFromEntity(EntityWrapper camera);
};