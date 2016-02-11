#include "Systems/PlayerSpawnSystem.h"

//This should be set by the config anyway.
float PlayerSpawnSystem::m_RespawnTime = 15.0f;

PlayerSpawnSystem::PlayerSpawnSystem(World* m_World, EventBroker* eventBroker) 
    : System(m_World, eventBroker)
    , m_Timer(0.f)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnInputCommand, &PlayerSpawnSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerSpawnerd, &PlayerSpawnSystem::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerDeath, &PlayerSpawnSystem::OnPlayerDeath);
    m_NetworkEnabled = ResourceManager::Load<ConfigFile>("Config.ini")->Get("Networking.StartNetwork", false);
}

void PlayerSpawnSystem::Update(double dt)
{
    //Increase timer.
    m_Timer += dt;
    if (m_Timer < m_RespawnTime) {
        return;
    }
    //If respawn time has passed, we spawn all players that have requested to be spawned.
    m_Timer = 0.f;

    //If there are no spawn requests, return immediately, if we are client the SpawnRequests should always be empty.
    if (m_SpawnRequests.size() == 0) {
        return;
    }

    auto playerSpawns = m_World->GetComponents("PlayerSpawn");
    if (playerSpawns == nullptr) {
        return;
    }

    int numSpawnedPlayers = 0;
    for (auto& req : m_SpawnRequests) {
        for (auto& cPlayerSpawn : *playerSpawns) {
            EntityWrapper spawner(m_World, cPlayerSpawn.EntityID);
            if (!spawner.HasComponent("Spawner")) {
                continue;
            }

            // If the spawner has a team affiliation, check it
            if (spawner.HasComponent("Team")) {
                if ((int)spawner["Team"]["Team"] != req.Team) {
                    continue;
                }
            }

            // Spawn the player!
            EntityWrapper player = SpawnerSystem::Spawn(spawner);
            // Set the player team affiliation
            player["Team"]["Team"] = req.Team;

            // Publish a PlayerSpawned event
            Events::PlayerSpawned e;
            e.PlayerID = req.PlayerID;
            e.Player = player;
            e.Spawner = spawner;
            m_EventBroker->Publish(e);
            ++numSpawnedPlayers;
            break;
        }
    }
    if (numSpawnedPlayers != (int)m_SpawnRequests.size()) {
        LOG_DEBUG("%i players were supposed to be spawned, but %i was spawned.", (int)m_SpawnRequests.size(), numSpawnedPlayers);
    } else {
        LOG_DEBUG("%i players were spawned.", numSpawnedPlayers);
    }
    m_SpawnRequests.clear();
}

bool PlayerSpawnSystem::OnInputCommand(Events::InputCommand& e)
{
    if (e.Command != "PickTeam") {
        return false;
    }

    // Team picks should be processed ONLY server-side!
    // Don't make a spawn request if PlayerID is -1, i.e. we're the client.
    if (e.PlayerID == -1 && m_NetworkEnabled) {
        return false;
    }

    if (e.Value == 0) {
        return false;
    }

    //TODO: Spectating?
    if (e.Player.Valid() && e.Player.HasComponent("Team")) {
        ComponentWrapper cTeam = e.Player["Team"];
        if ((ComponentInfo::EnumType)e.Value == cTeam["Team"].Enum("Spectator")) {
            return false;
        }
    }

    auto iter = m_SpawnRequests.begin();
    for (; iter != m_SpawnRequests.end(); ++iter) {
        if (iter->PlayerID == e.PlayerID) {
            break;
        }
    }

    if (iter != m_SpawnRequests.end()) {
        //If player is in queue to spawn, then change their team affiliation.
        iter->Team = (ComponentInfo::EnumType)e.Value;
    } else if (m_PlayerEntities.count(e.PlayerID) == 0 || !m_PlayerEntities[e.PlayerID].Valid()) {
        //If player is not in queue to spawn, then create a spawn request, 
        //but only if they are spectating and/or just connected.
        SpawnRequest req;
        req.PlayerID = e.PlayerID;
        req.Team = (ComponentInfo::EnumType)e.Value;
        m_SpawnRequests.push_back(req);
    } else {
        return false;
    }

    return true;
}

bool PlayerSpawnSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    // When a player is actually spawned (since the actual spawning is handled on the server)

    // Check if a player already exists
    if (m_PlayerEntities.count(e.PlayerID) != 0) {
        // TODO: Disallow infinite respawning here
        if (m_PlayerEntities[e.PlayerID].Valid()) {
            m_World->DeleteEntity(m_PlayerEntities[e.PlayerID].ID);
        }
    }

    // Store the player for future reference
    m_PlayerEntities[e.PlayerID] = e.Player;
    m_PlayerIDs[e.Player.ID] = e.PlayerID;

    // Set the camera to the correct entity
    EntityWrapper cameraEntity = e.Player.FirstChildByName("Camera");
    if (cameraEntity.Valid()) {
        Events::SetCamera e;
        e.CameraEntity = cameraEntity;
        m_EventBroker->Publish(e);
    }

    // HACK: Set the player model color to team color
    EntityWrapper playerModel = e.Player.FirstChildByName("PlayerModel");
    if (playerModel.Valid() && e.Player.HasComponent("Team")) {
        ComponentWrapper cTeam = e.Player["Team"];
        ComponentWrapper cModel = playerModel["Model"];
        if ((ComponentInfo::EnumType)cTeam["Team"] == cTeam["Team"].Enum("Red")) {
            cModel["Color"] = glm::vec3(1.f, 0.f, 0.f);
        } else if ((ComponentInfo::EnumType)cTeam["Team"] == cTeam["Team"].Enum("Blue")) {
            cModel["Color"] = glm::vec3(0.f, 0.25f, 1.f);
        }
    }

    // TODO: Set the player name to whatever
    EntityWrapper playerName = e.Player.FirstChildByName("PlayerName");
    if (playerName.Valid()) {
        playerName["Text"]["Content"] = e.PlayerName;
    }

    return true;
}

bool PlayerSpawnSystem::OnPlayerDeath(Events::PlayerDeath& e)
{
    if (!e.Player.HasComponent("Team")) {
        return false;
    }
    ComponentWrapper cTeam = e.Player["Team"];
    //A spectator can't die anyway
    if ((ComponentInfo::EnumType)cTeam["Team"] == cTeam["Team"].Enum("Spectator")) {
        return false;
    }
    SpawnRequest req;
    req.PlayerID = m_PlayerIDs[e.Player.ID];
    req.Team = cTeam["Team"];
    m_SpawnRequests.push_back(req);
}
