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
    SpawnerSystem(EventBroker* eventBroker);

private:
    EventRelay<SpawnerSystem, Events::SpawnerSpawn> m_OnSpawnerSpawn;
    bool OnSpawnerSpawn(Events::SpawnerSpawn& e);

    void spawnEntity(EntityWrapper spawner, EntityID parent, glm::vec3 position);
};