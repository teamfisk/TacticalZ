#include "Systems/PlayerSpawnSystem.h"
#include "Core/ELockMouse.h"

PlayerSpawnSystem::PlayerSpawnSystem(SystemParams params)
    : System(params)
    , m_DbgConfigForceRespawn(false)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnInputCommand, &PlayerSpawnSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerSpawnerd, &PlayerSpawnSystem::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerDeath, &PlayerSpawnSystem::OnPlayerDeath);
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    m_NetworkEnabled = config->Get("Networking.StartNetwork", false);
    m_ForcedRespawnTime = config->Get("Debug.RespawnTime", -1.0f);
    m_DbgConfigForceRespawn = m_ForcedRespawnTime > 0 && IsServer;
}

void PlayerSpawnSystem::Update(double dt)
{
    // If there are no CapturePointGameMode components we will just spawn immediately.
    // Should be able to support older maps with this.
    // TODO: In the future we might want to return instead, to avoid spawning in the menu for instance.
    auto pool = m_World->GetComponents("CapturePointGameMode");
    if (pool != nullptr && pool->size() > 0)
    {
        // Take the first CapturePointGameMode component found.
        ComponentWrapper& modeComponent = *pool->begin();
        // Increase timer.
        double& timer = (double&)modeComponent["RespawnTime"];
        timer += dt;
        if (m_DbgConfigForceRespawn) {
            (double&)modeComponent["MaxRespawnTime"] = m_ForcedRespawnTime;
        }
        double maxRespawnTime = (double)modeComponent["MaxRespawnTime"];
        EntityWrapper spectatorCam = m_World->GetFirstEntityByName("SpectatorCamera");
        if (spectatorCam.Valid()) {
            EntityWrapper HUD = spectatorCam.FirstChildByName("SpectatorHUD");
            if (HUD.Valid()) {
                EntityWrapper respawnTimer = spectatorCam.FirstChildByName("RespawnTimer");
                if (respawnTimer.Valid()) {
                    //Update respawn time in the HUD element.
                    respawnTimer["Text"]["Content"] = std::to_string(1 + (int)(maxRespawnTime - timer));
                }
            }
        }
        if (timer < maxRespawnTime) {
            return;
        }
        // If respawn time has passed, we spawn all players that have requested to be spawned.
        timer = 0;
    }

    // If there are no spawn requests, return immediately, if we are client the SpawnRequests should always be empty.
    if (m_SpawnRequests.size() == 0) {
        return;
    }

    auto playerSpawns = m_World->GetComponents("PlayerSpawn");
    if (playerSpawns == nullptr) {
        return;
    }

    int numSpawnedPlayers = 0;
    int playersSpectating = 0;
    const int numRequestsToHandle = (int)m_SpawnRequests.size();
    for (auto it = m_SpawnRequests.begin(); it != m_SpawnRequests.end(); ++it) {
        // TODO: -1 Signifies no class picked, or they are a spectator, enum here later?
        // It is valid if they didn't pick class yet
        // but don't spawn anything, goto next spawnrequest.
        if (it->Class == -1) {
            ++playersSpectating;
            continue;
        }
        for (auto& cPlayerSpawn : *playerSpawns) {
            EntityWrapper spawner(m_World, cPlayerSpawn.EntityID);
            if (!spawner.HasComponent("Spawner")) {
                continue;
            }

            // If the spawner has a team affiliation, check it
            if (spawner.HasComponent("Team")) {
                auto cSpawnerTeam = spawner["Team"];
                if ((int)cSpawnerTeam["Team"] != it->Team) {
                    continue;
                }
            }

            // TODO: Choose a different spawner depending on class picked?

            // Spawn the player!
            EntityWrapper player = SpawnerSystem::Spawn(spawner, EntityWrapper::Invalid, "Player");
            // Set the player team affiliation
            player["Team"]["Team"] = it->Team;

            // Publish a PlayerSpawned event
            Events::PlayerSpawned e;
            e.PlayerID = it->PlayerID;
            e.Player = player;
            e.Spawner = spawner;
            m_EventBroker->Publish(e);
            ++numSpawnedPlayers;
            it = m_SpawnRequests.erase(it);
            break;
        }
        if (it == m_SpawnRequests.end()) {
            break;
        }
    }
    if (numSpawnedPlayers != numRequestsToHandle - playersSpectating) {
        LOG_DEBUG("%i players were supposed to be spawned, but only %i was successfully.", numRequestsToHandle - playersSpectating, numSpawnedPlayers);
    } else {
        std::string dbg = numSpawnedPlayers != 0 ? std::to_string(numSpawnedPlayers) + " players were spawned. " : "";
        dbg += playersSpectating != 0 ? std::to_string(playersSpectating) + " players are spectating/picking class. " : "";
        LOG_DEBUG(dbg.c_str());
    }
}

bool PlayerSpawnSystem::OnInputCommand(Events::InputCommand& e)
{
    if (e.Command != "PickTeam" && e.Command != "PickClass" && e.Command != "SwapToTeamPick" && e.Command != "SwapToClassPick") {
        return false;
    }

    if (e.Value == 0) {
        return false;
    }

    // Team picks should be processed ONLY server-side!
    // Don't make a spawn request if we're the client.
    if (!IsServer && m_NetworkEnabled) {
        return false;
    }

    // Check if the player already requested spawn.
    auto iter = m_SpawnRequests.begin();
    for (; iter != m_SpawnRequests.end(); ++iter) {
        if (iter->PlayerID == e.PlayerID) {
            // If player wants to switch team or class , remove their selected class so they don't spawn.
            if (e.Command == "SwapToTeamPick" || e.Command == "SwapToClassPick") {
                iter->Class = -1;
                return true;
            }
            break;
        }
    }

    //If we get here we got a PickTeam or PickClass, so add or alter a spawn request.

    if (iter != m_SpawnRequests.end()) {
        // If player is in queue to spawn, then change their team affiliation or class in the request.
        if (e.Command == "PickTeam") {
            iter->Team = (ComponentInfo::EnumType)e.Value;
        } else {
            iter->Class = (ComponentInfo::EnumType)e.Value;
        }
    } else if (m_PlayerEntities.count(e.PlayerID) == 0 || !m_PlayerEntities[e.PlayerID].Valid()) {
        // If player is not in queue to spawn, then create a spawn request, 
        // but only if they are spectating and/or just connected.
        SpawnRequest req;
        req.PlayerID = e.PlayerID;
        if (e.Command == "PickTeam") {
            req.Team = (ComponentInfo::EnumType)e.Value;
            req.Class = -1;         // TODO: -1 Signifies no class picked, enum here later?
        } else {
            // Should never get here, since you should have picked a team before you ever get a chance to pick class.
            LOG_WARNING("Sequence error: Should not be able to pick class before team");
            req.Team = 1;           // TODO: 1 Signifies spectator, should probably have real enum here later.
            req.Class = (ComponentInfo::EnumType)e.Value;
        }
        m_SpawnRequests.push_back(req);
    } else {
        return false;
    }

    return true;
}

bool PlayerSpawnSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    // Store the player for future reference
    m_PlayerEntities[e.PlayerID] = e.Player;
    m_PlayerIDs[e.Player.ID] = e.PlayerID;

    // When a player is actually spawned (since the actual spawning is handled on the server)
    // Hack should be moved.

    // TODO: Set the player name to whatever
    EntityWrapper playerName = e.Player.FirstChildByName("PlayerName");
    if (playerName.Valid()) {
        playerName["Text"]["Content"] = e.PlayerName;
    }

    if (!IsClient) {
        return false;
    }

    // Set the camera to the correct entity
    EntityWrapper cameraEntity = e.Player.FirstChildByName("Camera");
    bool outOfBodyExperience = ResourceManager::Load<ConfigFile>("Config.ini")->Get<bool>("Debug.OutOfBodyExperience", false);
    if (cameraEntity.Valid() && !outOfBodyExperience) {
        Events::SetCamera e;
        e.CameraEntity = cameraEntity;
        m_EventBroker->Publish(e);
        Events::LockMouse lock;
        m_EventBroker->Publish(lock);
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

    return true;
}

bool PlayerSpawnSystem::OnPlayerDeath(Events::PlayerDeath& e)
{
    // Only spawn request if network is disabled or we are server.
    if (!IsServer && m_NetworkEnabled) {
        return false;
    }
    if (!e.Player.HasComponent("Team")) {
        return false;
    }
    ComponentWrapper cTeam = e.Player["Team"];
    // A spectator can't die anyway
    if ((ComponentInfo::EnumType)cTeam["Team"] == cTeam["Team"].Enum("Spectator")) {
        return false;
    }

    if (m_PlayerIDs.count(e.Player.ID) == 0) {
        return false;
    }

    SpawnRequest req;
    req.PlayerID = m_PlayerIDs.at(e.Player.ID);
    req.Team = cTeam["Team"];
    // TODO: Better than temp class state code.
    if (e.Player.HasComponent("DashAbility")) {
        req.Class = 1;      // Assault
    } else if (e.Player.HasComponent("SprintAbility")) {
        req.Class = 3;      // Sniper
    } else {
        req.Class = 2;      // Defender
    }
    m_SpawnRequests.push_back(req);

    return true;
}
