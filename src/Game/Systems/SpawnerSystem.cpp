#include "Systems/SpawnerSystem.h"

SpawnerSystem::SpawnerSystem(EventBroker* eventBroker) : System(eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnSpawnerSpawn, &SpawnerSystem::OnSpawnerSpawn);
}

bool SpawnerSystem::OnSpawnerSpawn(Events::SpawnerSpawn& e)
{
    EntityWrapper& spawner = e.Spawner;

    auto children = spawner.World->GetChildren(spawner.ID);
    std::vector<EntityID> spawnPoints;
    for (auto kv = children.first; kv != children.second; ++kv) {
        const EntityID& child = kv->second;
        if (spawner.World->HasComponent(child, "SpawnPoint")) {
            spawnPoints.push_back(child);
        }
    }

    EntityID spawnPoint = spawner.ID;
    if (!spawnPoints.empty()) {
        // Select a random spawn point
        static std::random_device randomDevice;
        static std::mt19937 randomGenerator(randomDevice());
        std::uniform_int_distribution<> distribution(0, std::distance(spawnPoints.begin(), spawnPoints.end()) - 1);
        auto randomSpawnPointIt = spawnPoints.begin();
        std::advance(randomSpawnPointIt, distribution(randomGenerator));
        spawnPoint = *randomSpawnPointIt;
    }
    spawnEntity(spawner, e.Parent.ID, Transform::AbsolutePosition(spawner.World, spawnPoint));

    return true;
}

void SpawnerSystem::spawnEntity(EntityWrapper spawner, EntityID parent, glm::vec3 position)
{
    const std::string& entityFilePath = spawner["Spawner"]["EntityFile"];
    auto entityFile = ResourceManager::Load<EntityFile>(entityFilePath);
    if (entityFile == nullptr) {
        return;
    }

    EntityFileParser parser(entityFile);
    EntityWrapper spawnedEntity(spawner.World, parser.MergeEntities(spawner.World, parent));
    spawnedEntity["Transform"]["Position"] = position;
}