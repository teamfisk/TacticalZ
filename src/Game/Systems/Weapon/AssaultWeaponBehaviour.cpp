#include "Systems/Weapon/AssaultWeaponBehaviour.h"

AssaultWeaponBehaviour::AssaultWeaponBehaviour(SystemParams systemParams, Octree<EntityAABB>* collisionOctree, EntityWrapper weaponEntity) 
    : WeaponBehaviour(systemParams, collisionOctree, weaponEntity)
{
    m_FirstPersonModel = m_Entity.FirstChildByName("Hands");
    EVENT_SUBSCRIBE_MEMBER(m_EAnimationComplete, &AssaultWeaponBehaviour::OnAnimationComplete);
}

void AssaultWeaponBehaviour::Fire()
{
    m_TimeSinceLastFire = 0.0;
    m_Firing = true;
    fireRound();
    playShootAnimation();
}

void AssaultWeaponBehaviour::CeaseFire()
{
    m_Firing = false;
}

void AssaultWeaponBehaviour::Reload()
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

void AssaultWeaponBehaviour::Update(double dt)
{
    if (m_Firing) {
        m_TimeSinceLastFire += dt;

        ComponentWrapper& cAssaultWeapon = m_Entity["AssaultWeapon"];
        if (m_TimeSinceLastFire >= 1.0 / ((double)cAssaultWeapon["RPM"] / 60.0)) {
            fireRound();
        }
    }

    if (!m_Firing && !m_Reloading) {
        playIdleAnimation();
    }
}

bool AssaultWeaponBehaviour::OnAnimationComplete(Events::AnimationComplete& e)
{
    if (e.Entity != m_FirstPersonModel) {
        return false;
    }

    //if (e.Name == "ShootRifle") {
    //    if (!m_Firing) {
    //        playIdleAnimation();
    //    }
    //}

    return true;
}

void AssaultWeaponBehaviour::fireRound()
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
    viewPunch();

    m_TimeSinceLastFire = 0.0;
}

void AssaultWeaponBehaviour::spawnTracer()
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

float AssaultWeaponBehaviour::traceRayDistance(glm::vec3 origin, glm::vec3 direction)
{
    // TODO: Cast a ray and size tracer appropriately
    return 100.f;
}

void AssaultWeaponBehaviour::playSound()
{
    if (!IsClient) {
        return;
    }

    Events::PlaySoundOnEntity e;
    e.EmitterID = m_Entity.ID;
    e.FilePath = "Audio/laser/laser1.wav";
    m_EventBroker->Publish(e);
}

void AssaultWeaponBehaviour::viewPunch()
{
    EntityWrapper playerCamera = m_Entity.FirstChildByName("Camera");
    if (!playerCamera.Valid()) {
        return;
    }
    float viewPunch = m_Entity["AssaultWeapon"]["ViewPunch"];
    ComponentWrapper cTransform = playerCamera["Transform"];
    glm::vec3& orientation = cTransform["Orientation"];
    orientation.x += viewPunch;
}

void AssaultWeaponBehaviour::playShootAnimation()
{
    EntityWrapper firstPersonWeapon = m_Entity.FirstChildByName("Hands");
    ComponentWrapper cAnimation = firstPersonWeapon["Animation"];
    cAnimation["AnimationName1"] = "ShootRifle";
    cAnimation["Weight1"] = 1.0;
    cAnimation["Time1"] = 0.0;
    cAnimation["Speed1"] = 1.0;
    cAnimation["Loop1"] = true;
}

void AssaultWeaponBehaviour::playIdleAnimation()
{
    if (!m_FirstPersonModel.Valid()) {
        return;
    }

    ComponentWrapper cAnimation = m_FirstPersonModel["Animation"];
    std::string& animationName1 = cAnimation["AnimationName1"];
    double& animationSpeed1 = cAnimation["Speed1"];

    std::string animationToPlay = "Idle";
    double speedToSet = 1.0;

    ComponentWrapper cPlayer = m_Entity["Player"];
    glm::vec3 movementDirection = cPlayer["CurrentWishDirection"];
    if (glm::length2(movementDirection) > 0) {
        animationToPlay = "Run";
        ComponentWrapper cPhysics = m_Entity["Physics"];
        speedToSet = glm::length((glm::vec3)cPhysics["Velocity"]) / (float)cPlayer["MovementSpeed"];
    }

    if (animationName1 != animationToPlay) {
        cAnimation["AnimationName1"] = animationToPlay;
        cAnimation["Weight1"] = 1.0;
        cAnimation["Time1"] = 0.0;
        cAnimation["Loop1"] = true;
    }

    if (animationSpeed1 != speedToSet) {
        cAnimation["Speed1"] = speedToSet;
    }
}

