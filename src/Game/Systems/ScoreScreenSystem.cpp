#include "Game/Systems/ScoreScreenSystem.h"


ScoreScreenSystem::ScoreScreenSystem(SystemParams params)
    : System(params)
    , PureSystem("ScoreScreen")
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDeath, &ScoreScreenSystem::OnPlayerDeath);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &ScoreScreenSystem::OnPlayerSpawn);
}

void ScoreScreenSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& capturePoint, double dt)
{
    //Logic here
}

void ScoreScreenSystem::OnPlayerDeath(const Events::PlayerDeath& e)
{
    //When player die, add it to his score, and when possible the player who killed him.
}

void ScoreScreenSystem::OnPlayerSpawn(const Events::PlayerSpawned& e)
{
    //When a player spawn, add a new entry to the score screen.
}
