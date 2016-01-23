#include "Systems/PlayerSpawnSystem.h"

PlayerSpawnSystem::PlayerSpawnSystem(World* m_World, EventBroker* eventBroker) 
    : System(m_World, eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnInputCommand, &PlayerSpawnSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerSpawnerd, &PlayerSpawnSystem::OnPlayerSpawned);
    m_NetworkEnabled = ResourceManager::Load<ConfigFile>("Config.ini")->Get("Networking.StartNetwork", false);
}

void PlayerSpawnSystem::Update(double dt)
{
    auto playerSpawns = m_World->GetComponents("PlayerSpawn");
    if (playerSpawns == nullptr) {
        return;
    }

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

        }
    }
    m_SpawnRequests.clear();
}

bool PlayerSpawnSystem::OnInputCommand(const Events::InputCommand& e)
{
    if (e.Command != "PickTeam") {
        return false;
    }

    // Team picks should be processed ONLY server-side!
    // Don't make a spawn request if PlayerID is -1, i.e. we're the client.
    if (e.PlayerID == -1 && m_NetworkEnabled) {
        return false;
    }

    if (e.Value != 0) {
        SpawnRequest req;
        req.PlayerID = e.PlayerID;
        req.Team = (ComponentInfo::EnumType)e.Value;
        m_SpawnRequests.push_back(req);
    }

    return true;
}

bool PlayerSpawnSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    // When a player is actually spawned (since the actual spawning is handled on the server)

    // Set the camera to the correct entity
    EntityWrapper cameraEntity = e.Player.FirstChildByName("Camera");
    if (cameraEntity.Valid()) {
        Events::SetCamera e;
        e.CameraEntity = cameraEntity;
        m_EventBroker->Publish(e);
    }

    return true;
}