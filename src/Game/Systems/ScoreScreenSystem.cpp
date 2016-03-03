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
    if(!entity.Valid() || !entity.HasComponent("ScoreScreen")){
        return;
    }

    auto children = entity.ChildrenWithComponent("ScoreIdentity");

    for (auto it = m_PlayerIdentities.begin(); it != m_PlayerIdentities.end(); ++it) {
        bool found = false;
        for (auto child : children) {
            std::string name = (std::string)child["ScoreIdentity"]["Name"];
            if (it->first == name) {
                found = true;
                break;
            }
        }
        if(found == false) {
            //There is no entry for this player, create one.

        }
    }
    //For each scoreboard, go through children and see if they have all of the ones needed. 
    //Also need to take into account the order so they dont flicker.
    //If they do not have all children we need to add them.
    //Just add a new component to the scorescreen entity, the "ScoreIdentity" component.
}

void ScoreScreenSystem::OnPlayerDeath(const Events::PlayerDeath& e)
{
    //When player die, add it to his score, and when possible the player who killed him.
}

void ScoreScreenSystem::OnPlayerSpawn(const Events::PlayerSpawned& e)
{
    //When a player spawn, add a new entry to the score screen.
    if(!e.Player.Valid()) {
        return;
    }
    EntityWrapper entity = e.Player;

    std::unordered_map<std::string, PlayerData>::const_iterator got;
    got = m_PlayerIdentities.find(e.PlayerName);
    if (got == m_PlayerIdentities.end()) {
        PlayerData data;
        data.ID = m_PlayerCounter;
        data.Name = e.PlayerName;
        data.Player = e.Player;
        if(!entity.HasComponent("Team")) {
            return;
        }
        data.Team = (int)entity["Team"]["Team"];

        std::pair<std::string, PlayerData> list (data.Name, data);
        m_PlayerIdentities.insert(list);
        m_PlayerCounter++;
    }

}
