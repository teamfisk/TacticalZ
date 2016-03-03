#include "Game/Systems/ScoreScreenSystem.h"


ScoreScreenSystem::ScoreScreenSystem(SystemParams params)
    : System(params)
    , PureSystem("ScoreScreen")
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDeath, &ScoreScreenSystem::OnPlayerDeath);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &ScoreScreenSystem::OnPlayerSpawn);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerConnected, &ScoreScreenSystem::OnPlayerConnected);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDisconnected, &ScoreScreenSystem::OnPlayerDisconnected);
}

void ScoreScreenSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& capturePoint, double dt)
{
    //TODO: Check team al
    if(!entity.HasComponent("ScoreScreen")){
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
            auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/ScoreIdentity.xml");
            EntityWrapper scoreIdentity = entityFile->MergeInto(m_World);
            auto cScoreIdentity = scoreIdentity["ScoreIdentity"];
            auto data = it->second;
            std::printf("\n\n");
            std::printf(data.Name.c_str());
            std::printf("\n\n");

            (std::string&)cScoreIdentity["Name"] = data.Name;
            (int&)cScoreIdentity["ID"] = data.ID;
            (int&)cScoreIdentity["Ping"] = 1337;

            m_World->SetName(scoreIdentity.ID, data.Name);
            m_World->SetParent(scoreIdentity.ID, entity.ID);
        }
    }
    //For each scoreboard, go through children and see if they have all of the ones needed. 
    //Also need to take into account the order so they dont flicker.
    //If they do not have all children we need to add them.
    //Just add a new component to the scorescreen entity, the "ScoreIdentity" component.
}

bool ScoreScreenSystem::OnPlayerDeath(const Events::PlayerDeath& e)
{
    //When player die, add it to his score, and when possible the player who killed him.
    return 0;
}

bool ScoreScreenSystem::OnPlayerSpawn(const Events::PlayerSpawned& e)
{
    //When a player spawn, add a new entry to the score screen.
    if(!e.Player.Valid()) {
        return 0;
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
            return 0;
        }
        data.Team = (int)entity["Team"]["Team"];

        std::pair<std::string, PlayerData> list (data.Name, data);
        m_PlayerIdentities.insert(list);
        m_PlayerCounter++;
    }
    return 0;
}

bool ScoreScreenSystem::OnPlayerConnected(const Events::PlayerConnected& e)
{
    return 0;
}

bool ScoreScreenSystem::OnPlayerDisconnected(const Events::PlayerDisconnected& e)
{
    return 0;
}
