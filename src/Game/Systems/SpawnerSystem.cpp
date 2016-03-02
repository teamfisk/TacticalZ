#include "Systems/SpawnerSystem.h"

SpawnerSystem::SpawnerSystem(SystemParams params) 
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnSpawnerSpawn, &SpawnerSystem::OnSpawnerSpawn);
}

EntityWrapper SpawnerSystem::Spawn(EntityWrapper spawner, EntityWrapper parent /*= EntityWrapper::Invalid*/)
{
    // Spawn the entity in the parent's world if it exists, otherwise in the spawner's world
    World* world = parent.World;
    if (world == nullptr) {
        world = spawner.World;
    } 

    // Find any SpawnPoints existing as children of spawner
    auto children = spawner.World->GetChildren(spawner.ID);
    std::vector<EntityWrapper> spawnPoints;
    for (auto kv = children.first; kv != children.second; ++kv) {
        const EntityID& child = kv->second;
        if (spawner.World->HasComponent(child, "SpawnPoint")) {
            spawnPoints.push_back(EntityWrapper(spawner.World, child));
        }
    }

    // Choose a random SpawnPoint
    EntityWrapper spawnPoint = spawner;
    if (!spawnPoints.empty()) {
        if (spawnPoints.size() > 1) {
            static std::random_device randomDevice;
            static std::mt19937 randomGenerator(randomDevice());
            std::uniform_int_distribution<> distribution(0, static_cast<int>(std::distance(spawnPoints.begin(), spawnPoints.end())) - 1);
            auto randomSpawnPointIt = spawnPoints.begin();
            std::advance(randomSpawnPointIt, distribution(randomGenerator));
            spawnPoint = *randomSpawnPointIt;
        } else {
            spawnPoint = spawnPoints.front();
        }
    }
    
    // Load the entity file and parse it
    const std::string& entityFilePath = spawner["Spawner"]["EntityFile"];
    auto entityFile = ResourceManager::Load<EntityFile>(entityFilePath);
    if (entityFile == nullptr) {
        return EntityWrapper::Invalid;
    }
    EntityFileParser parser(entityFile);
    EntityWrapper spawnedEntity(world, parser.MergeEntities(world, parent.ID));

    if (spawnPoint != parent) {
        // Set its position and orientation to that of the SpawnPoint
        spawnedEntity["Transform"]["Position"] = Transform::AbsolutePosition(spawnPoint.World, spawnPoint.ID);
        // TODO: Quaternions, bitch
        spawnedEntity["Transform"]["Orientation"] = glm::eulerAngles(Transform::AbsoluteOrientation(spawnPoint));
    }

    return spawnedEntity;
}

bool SpawnerSystem::OnSpawnerSpawn(Events::SpawnerSpawn& e)
{
    EntityWrapper spawnedEntity = Spawn(e.Spawner, e.Parent);
    return true;
}