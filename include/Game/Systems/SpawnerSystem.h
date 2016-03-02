#ifndef SpawnerSystem_h__
#define SpawnerSystem_h__

#include <random>
#include "Common.h"
#include "GLM.h"
#include "Core/System.h"
#include "Events/ESpawnerSpawn.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFileParser.h"

class SpawnerSystem : public System
{
public:
    SpawnerSystem(SystemParams params);

    static EntityWrapper Spawn(EntityWrapper spawner, EntityWrapper parent = EntityWrapper::Invalid);

private:
    EventRelay<SpawnerSystem, Events::SpawnerSpawn> m_OnSpawnerSpawn;
    bool OnSpawnerSpawn(Events::SpawnerSpawn& e);
};

#endif