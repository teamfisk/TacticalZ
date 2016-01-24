#include "Systems/WeaponSystem.h"

WeaponSystem::WeaponSystem(World* world, EventBroker* eventBroker, IRenderer* renderer)
    : System(world, eventBroker)
    , ImpureSystem()
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &WeaponSystem::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &WeaponSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EShoot, &WeaponSystem::OnShoot);
}

void WeaponSystem::Update(double dt)
{

}

bool WeaponSystem::OnPlayerSpawned(const Events::PlayerSpawned& e)
{
    if (e.PlayerID == -1) {
        m_LocalPlayer = e.Player;
    }
    return true;
}

bool WeaponSystem::OnInputCommand(const Events::InputCommand& e)
{
    // Only shoot client-side!
    if (e.PlayerID != -1) {
        return false;
    }

    // Only shoot if the player is alive
    if (!m_LocalPlayer.Valid()) {
        return false;
    }

    if (e.Command == "PrimaryFire" && e.Value > 0) {
        Events::Shoot eShoot;
        eShoot.Player = m_LocalPlayer;
        m_EventBroker->Publish(eShoot);
    }

    return true;
}

bool WeaponSystem::OnShoot(const Events::Shoot& eShoot) {
    // Screen center, based on current resolution!
    Rectangle screenResolution = m_Renderer->Resolution();
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
