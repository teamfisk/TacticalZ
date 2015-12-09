#ifndef World_h__
#define World_h__

#include "../Common.h"
#include "Entity.h"
#include "ObjectPool.h"
#include "ComponentPool.h"

class World
{
public:
    World() = default;
    ~World();

    // Create empty entity
    EntityID CreateEntity(EntityID parent = 0);

    // Register a component type and allocate space for it
    void RegisterComponent(ComponentInfo& ci);
    // Attach a component to an entity and fill it with default values
    ComponentWrapper AttachComponent(EntityID entity, std::string componentType);
    // Get a component of an entity
    ComponentWrapper GetComponent(EntityID entity, std::string componentType);
    // Get all components of the specified type
    const ComponentPool& GetComponents(std::string componentType);

private:
    EntityID m_CurrentEntityID = 1;

    std::unordered_map<EntityID, EntityID> m_EntityParents;
    std::unordered_multimap<EntityID, EntityID> m_EntityChildren;
    std::unordered_map<std::string, ComponentPool*> m_ComponentPools;

    EntityID generateEntityID();
};

#endif