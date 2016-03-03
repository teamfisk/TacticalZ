#ifndef SpawnerSystem_h__
#define SpawnerSystem_h__

#include <random>
#include "Common.h"
#include "GLM.h"
#include "Core/System.h"
#include "Events/ESpawnerSpawn.h"
#include "Core/Transform.h"
#include "Core/EntityFile.h"

class SpawnerSystem : public System
{
public:
    SpawnerSystem(SystemParams params);

    // If dontCollideComponent is set, to e.g. "Player", then all the spawner 
    // will try to pick a spawn location so that the spawned entity doesn't 
    // collide with anything that has that component and is collidable.
    static EntityWrapper Spawn(EntityWrapper spawner, EntityWrapper parent = EntityWrapper::Invalid, const std::string& dontCollideComponent = "");

private:
    EventRelay<SpawnerSystem, Events::SpawnerSpawn> m_OnSpawnerSpawn;
    bool OnSpawnerSpawn(Events::SpawnerSpawn& e);
    static void transformEntityToSpawnPoint(EntityWrapper spawnedEntity, EntityWrapper spawnPoint);
    static bool spawnedEntityIsColliding(EntityWrapper spawnedEntity, EntityWrapper spawnPoint, const std::string& dontCollideComponent);
};

#endif