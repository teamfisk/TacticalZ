#include "Systems/SpawnerSystem.h"
#include "Collision/Collision.h"

SpawnerSystem::SpawnerSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnSpawnerSpawn, &SpawnerSystem::OnSpawnerSpawn);
}

EntityWrapper SpawnerSystem::Spawn(EntityWrapper spawner, EntityWrapper parent /*= EntityWrapper::Invalid*/, const std::string& dontCollideComponent)
{
    // Spawn the entity in the parent's world if it exists, otherwise in the spawner's world
    World* world = parent.World;
    if (world == nullptr) {
        world = spawner.World;
    } 

    // Load the entity file and parse it
    const std::string& entityFilePath = spawner["Spawner"]["EntityFile"];
    EntityWrapper spawnedEntity;
    try {
        auto entityFile = ResourceManager::Load<EntityFile>(entityFilePath);
        spawnedEntity = entityFile->MergeInto(world);
        world->SetParent(spawnedEntity.ID, parent.ID);
    } catch (const Resource::FailedLoadingException& e) {
        return EntityWrapper::Invalid;
    }

    // If the spawned entity is collidable, then we must not spawn it where it collides with something that
    // has a dontCollideComponent attached.
    bool spawnOnCollidable = dontCollideComponent.empty() || !spawnedEntity.HasComponent("Collidable");
    if (!spawnOnCollidable) {
        boost::optional<EntityAABB> optBox = Collision::EntityAbsoluteAABB(spawnedEntity);
        //If we can't calculate the box for some reason, then just spawn somewhere anyway.
        if (!optBox) {
            spawnOnCollidable = true;
        }
    }

    // Find any SpawnPoints existing as children of spawner
    auto children = spawner.World->GetDirectChildren(spawner.ID);
    std::vector<EntityWrapper> spawnPoints;
    for (auto kv = children.first; kv != children.second; ++kv) {
        const EntityID& child = kv->second;
        if (spawner.World->HasComponent(child, "SpawnPoint")) {
            EntityWrapper spawnPoint = EntityWrapper(spawner.World, child);
            if (spawnOnCollidable || !spawnedEntityIsColliding(spawnedEntity, spawnPoint, dontCollideComponent)) {
                spawnPoints.push_back(spawnPoint);
            }
        }
    }

    // Choose a random SpawnPoint
    // If there are no children, or if they are all blocked, then the entity will be spawned at the spawner itself.
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
    
    if (spawnPoint != parent) {
        transformEntityToSpawnPoint(spawnedEntity, spawnPoint);
    }

    return spawnedEntity;
}

void SpawnerSystem::transformEntityToSpawnPoint(EntityWrapper spawnedEntity, EntityWrapper spawnPoint)
{
    // Set its position and orientation to that of the SpawnPoint
    spawnedEntity["Transform"]["Position"] = Transform::AbsolutePosition(spawnPoint.World, spawnPoint.ID);
    // TODO: Quaternions, bitch
    spawnedEntity["Transform"]["Orientation"] = glm::eulerAngles(Transform::AbsoluteOrientation(spawnPoint));
}

bool SpawnerSystem::spawnedEntityIsColliding(EntityWrapper spawnedEntity, EntityWrapper spawnPoint, const std::string& dontCollideComponent)
{
    transformEntityToSpawnPoint(spawnedEntity, spawnPoint);
    //Check if the spawned entity collides with anything, and if so, continue to the next spawnpoint.
    EntityAABB spawnedBox = *Collision::EntityAbsoluteAABB(spawnedEntity);
    auto otherSpawnedEntities = spawnPoint.World->GetComponents(dontCollideComponent);
    for (const auto& obj : *otherSpawnedEntities) {
        if (spawnedEntity.ID == obj.EntityID) {
            continue;
        }
        EntityWrapper otherEntity = EntityWrapper(spawnPoint.World, obj.EntityID);
        if (!otherEntity.HasComponent("Collidable")) {
            continue;
        }
        auto otherBox = Collision::EntityAbsoluteAABB(otherEntity);
        if (!otherBox) {
            continue;
        }
        if (Collision::AABBVsAABB(spawnedBox, *otherBox)) {
            if (!spawnedBox.Entity.HasComponent("Model")) {
                return true;
            }
            RawModel* model = nullptr;
            try {
                model = ResourceManager::Load<RawModel, true>(otherEntity["Model"]["Resource"]);
            } catch (const std::exception&) {
            }

            if (model != nullptr && Collision::AABBvsTriangles(
                    spawnedBox,
                    model->Vertices(),
                    model->m_Indices,
                    Transform::ModelMatrix(otherEntity))) {
                return true;
            }
        }
    }
    return false;
}

bool SpawnerSystem::OnSpawnerSpawn(Events::SpawnerSpawn& e)
{
    EntityWrapper spawnedEntity = Spawn(e.Spawner, e.Parent);
    return true;
}