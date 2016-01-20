#ifndef ESpawnerSpawn_h__
#define ESpawnerSpawn_h__

#include "Core/Event.h"
#include "Core/EntityWrapper.h"

namespace Events
{

struct SpawnerSpawn : Event
{
    EntityWrapper Spawner;
    EntityWrapper Parent;
};

}

#endif