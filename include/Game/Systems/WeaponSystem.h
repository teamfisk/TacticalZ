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
#include "Core/Octree.h"
#include "Collision/EntityAABB.h"
#include "Systems/SpawnerSystem.h"
#include "Sound/EPlaySoundOnEntity.h"

class WeaponBehaviour;

class WeaponSystem : public PureSystem, ImpureSystem
{
public:
    WeaponSystem(SystemParams params, IRenderer* renderer, Octree<EntityAABB>* collisionOctree);

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cPlayer, double dt) override;

private:
    SystemParams m_SystemParams;
    IRenderer* m_Renderer;
    Octree<EntityAABB>* m_CollisionOctree;

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

class AssaultWeaponBehaviour : public WeaponBehaviour
{
public:
    AssaultWeaponBehaviour(SystemParams systemParams, Octree<EntityAABB>* collisionOctree, EntityWrapper weaponEntity)
        : WeaponBehaviour(systemParams, collisionOctree, weaponEntity)
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

    virtual void Reload() override
    {
        ComponentWrapper& cAssaultWeapon = m_Entity["AssaultWeapon"];

        int& magAmmo = cAssaultWeapon["MagazineAmmo"];
        int magSize = cAssaultWeapon["MagazineSize"];
        int& ammo = cAssaultWeapon["Ammo"];

        // Don't reload if we're already fully loaded
        if (magAmmo == magSize) {
            return;
        }

        // Throw away rounds in magazine to incentivise ammo sharing
        int toLoad = glm::min(magSize, ammo);
        magAmmo = toLoad;
        ammo -= toLoad;
    }

    virtual void Update(double dt) override
    {
        if (!m_Firing) {
            return;
        }

        m_TimeSinceLastFire += dt;

        ComponentWrapper& cAssaultWeapon = m_Entity["AssaultWeapon"];
        if (m_TimeSinceLastFire >= 1.0 / ((double)cAssaultWeapon["RPM"] / 60.0)) {
            fireRound();
        }
    }

private:
    bool m_Firing = false;
    double m_TimeSinceLastFire = 0.0;
    EntityFile* m_RayRed = nullptr;
    EntityFile* m_RayBlue = nullptr;

    void fireRound()
    {
        ComponentWrapper& cAssaultWeapon = m_Entity["AssaultWeapon"];

        int& magAmmo = cAssaultWeapon["MagazineAmmo"];
        int ammo = cAssaultWeapon["Ammo"];

        // Reload if our magazine is empty
        if (magAmmo <= 0) {
            Reload();
            return;
        }

        // Fire
        magAmmo -= 1;
        spawnTracer();
        playSound();

        m_TimeSinceLastFire = 0.0;
    }

    void spawnTracer()
    {
        if (!IsClient) {
            return;
        }

        EntityWrapper spawner;
        if (m_Entity == LocalPlayer) {
            spawner = m_Entity.FirstChildByName("WeaponMuzzle");
        } else {
            spawner = m_Entity.FirstChildByName("ThirdPersonWeaponMuzzle");
        }

        if (!spawner.Valid()) {
            return;
        }

        Events::SpawnerSpawn e;
        e.Spawner = spawner;
        m_EventBroker->Publish(e);
    }

    float traceRayDistance(glm::vec3 origin, glm::vec3 direction)
    {
        // TODO: Cast a ray and size tracer appropriately
        return 100.f;
    }

    void playSound()
    { 
        if (!IsClient) {
            return;
        }

        Events::PlaySoundOnEntity e;
        e.EmitterID = m_Entity.ID;
        e.FilePath = "Audio/laser/laser1.wav";
        m_EventBroker->Publish(e);
    }
};

#endif