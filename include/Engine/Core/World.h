#ifndef World_h__
#define World_h__

#include "../Common.h"
#include "Entity.h"
#include "ComponentPool.h"

class World
{
public:
	World()
	{

	}

    ~World()
    {
        // foreach (m_ComponentPools...
    }

    void AllocateComponentPool(ComponentInfo& ci)
    {
        auto pool = new ComponentPool(ci);
        m_ComponentPools[ci.Name] = pool;
    }

    void AddComponent(EntityID entity, std::string componentType);

private:
    std::unordered_map<std::string, ComponentPool*> m_ComponentPools;
};

#endif