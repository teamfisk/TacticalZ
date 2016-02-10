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

class WeaponBehaviour : protected System
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
    { 
        m_RayRed = ResourceManager::Load<EntityFile>("Schema/Entities/RayRed.xml");
        m_RayBlue = ResourceManager::Load<EntityFile>("Schema/Entities/RayBlue.xml");
    }
    
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

        m_TimeSinceLastFire = 0.0;
    }

    void spawnTracer()
    {
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

        //ComponentWrapper cTeam = m_Entity["Team"];
        //ComponentInfo::EnumType team = cTeam["Team"];

        //// Select the right color of effect
        //EntityFile* rayFile = nullptr;
        //if (team == cTeam["Team"].Enum("Red")) {
        //    rayFile = m_RayRed;
        //}
        //if (team == cTeam["Team"].Enum("Blue")) {
        //    rayFile = m_RayBlue;
        //}
        //if (rayFile == nullptr) {
        //    return;
        //}

        //// Create the entity
        //EntityFileParser parser(rayFile);
        //EntityID rayID = parser.MergeEntities(m_World);
        //EntityWrapper ray(m_World, rayID);

        //// Figure out where to put it
        //EntityWrapper attachment;
        //if (m_Entity == LocalPlayer || true) {
        //    // Spawn the effect from the weapon view model for the local player
        //    attachment = m_Entity.FirstChildByName("WeaponMuzzle");
        //}
        //// TODO: Spawn the effect from the weapon world model once it exists

        //glm::mat4 transformation = Transform::AbsoluteTransformation(attachment);
        //glm::vec3 _scale;
        //glm::vec3 translation;
        //glm::quat _orientation;
        //glm::vec3 _skew;
        //glm::vec4 _perspective;
        //glm::decompose(transformation, _scale, _orientation, translation, _skew, _perspective);
        //
        //// Matrix to euler angles
        //glm::vec3 euler;
        //euler.y = glm::asin(-transformation[0][2]);
        //if (cos(euler.y) != 0) {
        //    euler.x = atan2(transformation[1][2], transformation[2][2]);
        //    euler.z = atan2(transformation[0][1], transformation[0][0]);
        //} else {
        //    euler.x = atan2(-transformation[2][0], transformation[1][1]);
        //    euler.z = 0;
        //}

        //// TODO: Spread?

        //(glm::vec3&)ray["Transform"]["Position"] = translation;
        //(glm::vec3&)ray["Transform"]["Orientation"] = euler;
        //glm::vec3& scale = ray["Transform"]["Scale"];
        //scale.z = traceRayDistance(translation, glm::quat(euler) * glm::vec3(0.f, 0.f, -1.f));
    }

    float traceRayDistance(glm::vec3 origin, glm::vec3 direction)
    {
        // TODO: Cast a ray and size tracer appropriately
        return 100.f;
    }
};

#endif