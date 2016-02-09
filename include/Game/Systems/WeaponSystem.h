#ifndef WeaponSystem_h__
#define WeaponSystem_h__

//#include <GLFW/glfw3.h>
//#include <glm/common.hpp>
#include "Rendering/IRenderer.h"

#include "Common.h"
#include "Core/System.h"
#include "Core/EPlayerDamage.h"
#include "Core/EShoot.h"
#include "Core/EPlayerSpawned.h"
#include "Input/EInputCommand.h"
#include "Core/EntityFile.h"
#include "Core/EntityFileParser.h"

class WeaponSystem : public PureSystem, ImpureSystem
{
public:
    WeaponSystem(SystemParams params, IRenderer* renderer);

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt) override;

private:
    IRenderer* m_Renderer;

    std::unordered_map<EntityWrapper, std::shared_ptr<WeaponBehaviour>> m_ActiveWeapons;

    // Events
    EventRelay<WeaponSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(Events::PlayerSpawned& e);
    EventRelay<WeaponSystem, Events::Shoot> m_EShoot;
    bool OnShoot(Events::Shoot& e);
    EventRelay<WeaponSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(Events::InputCommand& e);

    void selectWeapon(EntityWrapper player, ComponentInfo::EnumType slot);
};

class WeaponBehaviour
{
public:
    WeaponBehaviour(EntityWrapper weaponEntity)
        : m_Entity(weaponEntity)
    { }

    virtual void Fire() = 0;
    virtual void CeaseFire() { }
    virtual void Reload() { }
    virtual void Update(double dt) { }

protected:
    EntityWrapper m_Entity;
};

class AssaultWeaponBehaviour : public WeaponBehaviour
{
public:
    AssaultWeaponBehaviour(EntityWrapper weaponEntity)
        : WeaponBehaviour(weaponEntity)
    { }
    
    virtual void Fire() override
    {
        m_TimeSinceLastFire = 0.0;
        m_Firing = true;
        fireRound();
    }

    virtual void CeaseFire() override
    {
        m_Firing = false;
    }

    virtual void Update(double dt) override
    {
        if (!m_Firing) {
            return;
        }

        m_TimeSinceLastFire += dt;

        ComponentWrapper& cAssaultWeapon = m_Entity["AssaultWeapon"];
        if (m_TimeSinceLastFire > (double)cAssaultWeapon["RPM"] / 60.0) {
            fireRound();
        }
    }

private:
    bool m_Firing = false;
    double m_TimeSinceLastFire = 0.0;
    ComponentWrapper m_Component;

    void fireRound()
    {
        ComponentWrapper& cAssaultWeapon = m_Entity["AssaultWeapon"];

        int& magAmmo = cAssaultWeapon["MagazineAmmo"];
        int ammo = cAssaultWeapon["Ammo"];

        // Reload if our magazine is empty and we have ammo to fill it with
        if (magAmmo <= 0 && ammo > 0) {
            CeaseFire();
            Reload();
            return;
        }

        // Fire
        magAmmo -= 1;

        m_TimeSinceLastFire = 0.0;
    }
};

#endif