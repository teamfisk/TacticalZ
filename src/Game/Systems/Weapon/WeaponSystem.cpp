#include "Systems/Weapon/WeaponSystem.h"

WeaponSystem::WeaponSystem(SystemParams params, IRenderer* renderer, Octree<EntityAABB>* collisionOctree)
    : System(params)
    , PureSystem("Player")
    , m_SystemParams(params)
    , m_Renderer(renderer)
    , m_CollisionOctree(collisionOctree)
{
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &WeaponSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EShoot, &WeaponSystem::OnShoot);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &WeaponSystem::OnPlayerSpawned);
}

void WeaponSystem::Update(double dt)
{

}

void WeaponSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cPlayer, double dt)
{
    // Update potential weapon behaviour for player
    auto it = m_ActiveWeapons.find(entity);
    if (it == m_ActiveWeapons.end()) {
        selectWeapon(entity, 1);
    }
    
    m_EventBroker->Process<WeaponBehaviour>();
    m_ActiveWeapons.at(entity)->Update(dt);
}

bool WeaponSystem::OnInputCommand(Events::InputCommand& e)
{
    EntityWrapper player = e.Player;
    if (e.PlayerID == -1) {
        player = LocalPlayer;
    }

    // Make sure player is alive
    if (!player.Valid()) {
        return false;
    }

    // Weapon selection
    if (e.Command == "SelectWeapon") {
        if (e.Value != 0) {
            //selectWeapon(player, static_cast<ComponentInfo::EnumType>(e.Value));
        }
    }

    // Fire
    if (e.Command == "PrimaryFire") {
        if (m_ActiveWeapons.find(player) != m_ActiveWeapons.end()) {
            auto weapon = m_ActiveWeapons.at(player);
            if (e.Value > 0) {
                weapon->Fire();
            } else {
                weapon->CeaseFire();
            }
        }
    }

    return true;
}

void WeaponSystem::selectWeapon(EntityWrapper player, ComponentInfo::EnumType slot)
{
    // Primary
    if (slot == 1) {
        // TODO: if class...
        if (m_ActiveWeapons.count(player) == 0) {
            m_ActiveWeapons.insert(std::make_pair(player, std::make_shared<AssaultWeaponBehaviour>(m_SystemParams, m_CollisionOctree, player)));
        } else {
            //m_ActiveWeapons.erase(player);
        }
    }

    // Secondary
    if (slot == 2) {
        //m_ActiveWeapons[player] = std::make_shared<PistolWeaponBehaviour>();
    }
}

bool WeaponSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    // Select primary weapon on player spawn
    // TODO: Select the active one specified by player component
    return true;
}

bool WeaponSystem::OnShoot(Events::Shoot& eShoot)
{
    if (!eShoot.Player.Valid()) {
        return false;
    }

    // Only run further picking code for the local player!
    if (eShoot.Player != LocalPlayer) {
        return false;
    }

    // Screen center, based on current resolution!
    // TODO: check if player has enough ammo and if weapon has a cooldown or not
    Rectangle screenResolution = m_Renderer->GetViewportSize();
    glm::vec2 centerScreen = glm::vec2(screenResolution.Width / 2, screenResolution.Height / 2);

    // TODO: check if player has enough ammo and if weapon has a cooldown or not

    // Pick middle of screen
    PickData pickData = m_Renderer->Pick(centerScreen);
    if (pickData.Entity == EntityID_Invalid) {
        return false;
    }

    EntityWrapper player(m_World, pickData.Entity);

    // Only care about players being hit
    if (!player.HasComponent("Player")) {
        player = player.FirstParentWithComponent("Player");
    }
    if (!player.Valid()) {
        return false;
    }

    // Check for friendly fire
    EntityWrapper shooter = eShoot.Player;
    if ((ComponentInfo::EnumType)player["Team"]["Team"] == (ComponentInfo::EnumType)shooter["Team"]["Team"]) {
        return false;
    }

    // TODO: Weapon damage calculations etc
    Events::PlayerDamage ePlayerDamage;
    ePlayerDamage.Player = player;
    ePlayerDamage.Damage = 100;
    m_EventBroker->Publish(ePlayerDamage);

    return true;
}