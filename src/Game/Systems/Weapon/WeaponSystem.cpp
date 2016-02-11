#include "Systems/Weapon/WeaponSystem.h"

WeaponSystem::WeaponSystem(SystemParams params, IRenderer* renderer, Octree<EntityAABB>* collisionOctree)
    : System(params)
    , PureSystem("Player")
    , m_SystemParams(params)
    , m_Renderer(renderer)
    , m_CollisionOctree(collisionOctree)
{
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &WeaponSystem::OnInputCommand);
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

    // Reload
    if (e.Command == "Reload" && e.Value != 0) {
        if (m_ActiveWeapons.find(player) != m_ActiveWeapons.end()) {
            auto weapon = m_ActiveWeapons.at(player);
            weapon->Reload();
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
            m_ActiveWeapons.insert(std::make_pair(player, std::make_shared<AssaultWeaponBehaviour>(m_SystemParams, m_Renderer, m_CollisionOctree, player)));
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
