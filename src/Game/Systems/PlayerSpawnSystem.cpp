#include "Systems/PlayerSpawnSystem.h"

PlayerSpawnSystem::PlayerSpawnSystem(EventBroker* eventBroker) 
    : System(eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnInputCommand, &PlayerSpawnSystem::OnInputCommand);
}

void PlayerSpawnSystem::Update(World* world, double dt)
{
    auto componentPools = world->GetComponentPools();
    auto spawnerPool = componentPools.find("Spawner");
    if (spawnerPool == componentPools.end()) {
        return;
    }
;
    for (auto& team : m_SpawnRequests) {
        for (auto& spawner : *spawnerPool->second) {
            Events::SpawnerSpawn e;
            e.Spawner = EntityWrapper(world, spawner.EntityID);
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

    if (e.Value != 0) {
        m_SpawnRequests.push_back((int)e.Value);
    }

    return true;
}

