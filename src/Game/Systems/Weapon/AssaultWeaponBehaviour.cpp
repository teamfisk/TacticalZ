#include "Systems/Weapon/AssaultWeaponBehaviour.h"

AssaultWeaponBehaviour::AssaultWeaponBehaviour(SystemParams systemParams, IRenderer* renderer, Octree<EntityAABB>* collisionOctree, EntityWrapper player) 
    : WeaponBehaviour(systemParams, renderer, collisionOctree, player)
{
    m_FirstPersonModel = m_Player.FirstChildByName("Hands");
    m_ThirdPersonModel = m_Player.FirstChildByName("PlayerModel");
    EVENT_SUBSCRIBE_MEMBER(m_EAnimationComplete, &AssaultWeaponBehaviour::OnAnimationComplete);
}

void AssaultWeaponBehaviour::Fire()
{
    m_TimeSinceLastFire = 0.0;
    m_Firing = true;
    fireRound();
}

void AssaultWeaponBehaviour::CeaseFire()
{
    m_Firing = false;
}

void AssaultWeaponBehaviour::Reload()
{
    if (m_Reloading) {
        return;
    }

    ComponentWrapper& cAssaultWeapon = m_Player["AssaultWeapon"];
    int magAmmo = cAssaultWeapon["MagazineAmmo"];
    int magSize = cAssaultWeapon["MagazineSize"];
    int ammo = cAssaultWeapon["Ammo"];

    // Don't reload if we're already fully loaded
    if (magAmmo == magSize) {
        return;
    }

    // Don't reload if we're completly out of ammo
    if (ammo == 0) {
        return;
    }

    m_Reloading = true;
    m_ReloadTimer = cAssaultWeapon["ReloadTime"];
    playReloadAnimation();
}

void AssaultWeaponBehaviour::Update(double dt)
{
    if (m_Reloading) {
        m_ReloadTimer -= dt;
        // Re-enable glow on reload impersonator half-way through the animation
        if (IsClient) {
            if (m_ReloadTimer <= (double)m_Player["AssaultWeapon"]["ReloadTime"] / 2.0) {
                if (m_FirstPersonReloadImpersonator.Valid()) {
                    m_FirstPersonReloadImpersonator["Model"]["GlowMap"] = true;
                }
                if (m_ThirdPersonReloadImpersonator.Valid()) {
                    m_ThirdPersonReloadImpersonator["Model"]["GlowMap"] = true;
                }
            }
        }
        if (m_ReloadTimer <= 0) {
            finishReload();
        }
    }

    if (m_Firing && !m_Reloading) {
        m_TimeSinceLastFire += dt;
        ComponentWrapper& cAssaultWeapon = m_Player["AssaultWeapon"];
        if (m_TimeSinceLastFire >= 1.0 / ((double)cAssaultWeapon["RPM"] / 60.0)) {
            fireRound();
        }
    }

    if (!m_Firing && !m_Reloading) {
        if (IsClient) {
            playIdleAnimation();
        }
    }

    // Disable glow map on weapon if it's out of ammo
    // Make real first person weapon model visible again
    if (IsClient) {
        EntityWrapper firstPersonWeaponModel = m_Player.FirstChildByName("WeaponModel");
        if (firstPersonWeaponModel.Valid()) {
            firstPersonWeaponModel["Model"]["GlowMap"] = hasAmmo();
        }
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

bool AssaultWeaponBehaviour::hasAmmo()
{
    ComponentWrapper& cAssaultWeapon = m_Player["AssaultWeapon"];
    int& magAmmo = cAssaultWeapon["MagazineAmmo"];
    return magAmmo > 0;
}

void AssaultWeaponBehaviour::fireRound()
{
    if (m_Reloading) {
        return;
    }

    ComponentWrapper& cAssaultWeapon = m_Player["AssaultWeapon"];

    int& magAmmo = cAssaultWeapon["MagazineAmmo"];
    int ammo = cAssaultWeapon["Ammo"];

    // Reload if our magazine is empty
    if (magAmmo <= 0) {
        Reload();
        return;
    }

    // Fire
    magAmmo -= 1;
    m_TimeSinceLastFire = 0.0;

    // Effects
    if (IsClient) {
        spawnTracer();
        playSound();
        viewPunch();
        playShootAnimation();
        bool hit = shoot(cAssaultWeapon["BaseDamage"]);
        if (hit) {
            showHitMarker();
        }
    }
}

void AssaultWeaponBehaviour::spawnTracer()
{
    if (!IsClient) {
        return;
    }

    EntityWrapper spawner;
    bool outOfBodyExperience = ResourceManager::Load<ConfigFile>("Config.ini")->Get<bool>("Debug.OutOfBodyExperience", false);
    if (m_Player == LocalPlayer && !outOfBodyExperience) {
        spawner = m_Player.FirstChildByName("WeaponMuzzle");
    } else {
        spawner = m_Player.FirstChildByName("ThirdPersonWeaponMuzzle");
    }

    if (!spawner.Valid()) {
        return;
    }

    float distance = traceRayDistance(Transform::AbsolutePosition(spawner), Transform::AbsoluteOrientation(spawner) * glm::vec3(0, 0, -1));
    EntityWrapper ray = SpawnerSystem::Spawn(spawner);
    ((glm::vec3&)ray["Transform"]["Scale"]).z = (distance / 100.f);
}

float AssaultWeaponBehaviour::traceRayDistance(glm::vec3 origin, glm::vec3 direction)
{
    // TODO: Cast a ray and size tracer appropriately
    float distance;
    glm::vec3 pos;
    auto entity = Collision::EntityFirstHitByRay(Ray(origin, direction), m_CollisionOctree, distance, pos);
    if (entity) {
        return distance;
    } else {
        return 100.f;
    }
}

void AssaultWeaponBehaviour::playSound()
{
    if (!IsClient) {
        return;
    }

    Events::PlaySoundOnEntity e;
    e.EmitterID = m_Player.ID;
    e.FilePath = "Audio/laser/laser1.wav";
    m_EventBroker->Publish(e);
}

void AssaultWeaponBehaviour::viewPunch()
{
    EntityWrapper playerCamera = m_Player.FirstChildByName("Camera");
    if (!playerCamera.Valid()) {
        return;
    }
    float viewPunch = m_Player["AssaultWeapon"]["ViewPunch"];
    ComponentWrapper cTransform = playerCamera["Transform"];
    glm::vec3& orientation = cTransform["Orientation"];
    orientation.x += viewPunch;
}

void AssaultWeaponBehaviour::finishReload()
{
    ComponentWrapper& cAssaultWeapon = m_Player["AssaultWeapon"];
    int& magAmmo = cAssaultWeapon["MagazineAmmo"];
    int magSize = cAssaultWeapon["MagazineSize"];
    int& ammo = cAssaultWeapon["Ammo"];

    // Throw away rounds in magazine to incentivise ammo sharing
    int toLoad = glm::min(magSize, ammo);
    magAmmo = toLoad;
    ammo -= toLoad;

    // Make real first person weapon model visible again
    EntityWrapper firstPersonWeaponModel = m_Player.FirstChildByName("WeaponModel");
    if (firstPersonWeaponModel.Valid()) {
        firstPersonWeaponModel["Model"]["Visible"] = true;
    }
    EntityWrapper thirdPersonWeaponModel = m_Player.FirstChildByName("ThirdPersonWeaponModel");
    if (thirdPersonWeaponModel.Valid()) {
        thirdPersonWeaponModel["Model"]["Visible"] = true;
    }

    m_Reloading = false;
}

void AssaultWeaponBehaviour::playShootAnimation()
{
    ComponentWrapper cAnimation = m_FirstPersonModel["Animation"];
    if (cAnimation["AnimationName1"] != "ShootRifle") {
        cAnimation["AnimationName1"] = "ShootRifle";
        cAnimation["Weight1"] = 1.0;
        cAnimation["Time1"] = 0.0;
        cAnimation["Speed1"] = 1.0;
        cAnimation["Loop1"] = true;
    }
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

    ComponentWrapper cPlayer = m_Player["Player"];
    glm::vec3 movementDirection = cPlayer["CurrentWishDirection"];
    if (glm::length2(movementDirection) > 0) {
        animationToPlay = "Run";
        ComponentWrapper cPhysics = m_Player["Physics"];
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

void AssaultWeaponBehaviour::playReloadAnimation()
{
    // Play animation
    // First person
    if (IsClient)
    {
        ComponentWrapper cAnimation = m_FirstPersonModel["Animation"];
        cAnimation["AnimationName1"] = "ReloadSwitch";
        cAnimation["Weight1"] = 1.0;
        cAnimation["Time1"] = 0.0;
        cAnimation["Speed1"] = 0.5;
        cAnimation["Loop1"] = true;
    }
    // TODO: Third person
    //{
    //    ComponentWrapper cAnimation = m_ThirdPersonModel["Animation"];
    //    cAnimation["AnimationName1"] = "ReloadSwitch";
    //    cAnimation["Weight1"] = 1.0;
    //    cAnimation["Time1"] = 0.0;
    //    cAnimation["Speed1"] = 0.5;
    //    cAnimation["Loop1"] = true;
    //}

    // Hide weapon model and spawn the exploding version
    {
        EntityWrapper firstPersonWeaponModel = m_Player.FirstChildByName("WeaponModel");
        EntityWrapper reloadSpawner = m_Player.FirstChildByName("FirstPersonReloadSpawner");
        if (IsClient) {
            m_FirstPersonReloadImpersonator = SpawnerSystem::Spawn(reloadSpawner, reloadSpawner);
            firstPersonWeaponModel["Model"].Copy(m_FirstPersonReloadImpersonator["Model"]);
        }
        firstPersonWeaponModel["Model"]["Visible"] = false;
    }
    {
        EntityWrapper thirdPersonWeaponModel = m_Player.FirstChildByName("ThirdPersonWeaponModel");
        EntityWrapper reloadSpawner = m_Player.FirstChildByName("ThirdPersonReloadSpawner");
        if (IsClient) {
            m_ThirdPersonReloadImpersonator = SpawnerSystem::Spawn(reloadSpawner, reloadSpawner);
            thirdPersonWeaponModel["Model"].Copy(m_ThirdPersonReloadImpersonator["Model"]);
        }
        thirdPersonWeaponModel["Model"]["Visible"] = false;
    }
}

bool AssaultWeaponBehaviour::shoot(double damage)
{
    // Only do shooting clientside
    if (!IsClient) {
        return false;
    }

    // Only handle shooting for the local player
    if (m_Player != LocalPlayer) {
        return false;
    }

    // Make sure the player isn't shooting from the grave
    if (!m_Player.Valid()) {
        return false;
    }

    // Screen center, based on current resolution!
    Rectangle screenResolution = m_Renderer->GetViewportSize();
    glm::vec2 centerScreen = glm::vec2(screenResolution.Width / 2, screenResolution.Height / 2);

    // Pick middle of screen
    PickData pickData = m_Renderer->Pick(centerScreen);
    if (pickData.Entity == EntityID_Invalid) {
        return false;
    }

    EntityWrapper victim(m_World, pickData.Entity);

    // Don't let us shoot ourselves in the foot
    if (victim == LocalPlayer) {
        return false;
    }

    // Only care about players being hit
    if (!victim.HasComponent("Player")) {
        victim = victim.FirstParentWithComponent("Player");
    }
    if (!victim.Valid()) {
        return false;
    }

    // Check for friendly fire
    if ((ComponentInfo::EnumType)victim["Team"]["Team"] == (ComponentInfo::EnumType)m_Player["Team"]["Team"]) {
        return false;
    }

    // Deal damage! 
    Events::PlayerDamage ePlayerDamage;
    ePlayerDamage.Inflictor = m_Player;
    ePlayerDamage.Victim = victim;
    ePlayerDamage.Damage = damage;
    m_EventBroker->Publish(ePlayerDamage);

    return true;
}

void AssaultWeaponBehaviour::showHitMarker()
{
    // Show hit marker
    EntityWrapper hitMarkerSpawner = m_Player.FirstChildByName("HitMarkerSpawner");
    if (hitMarkerSpawner.Valid()) {
        SpawnerSystem::Spawn(hitMarkerSpawner, hitMarkerSpawner);
    }
}
