#ifndef World_h__
#define World_h__

#include "../Common.h"
#include "Entity.h"
#include "ObjectPool.h"
#include "ComponentPool.h"
#include "EventBroker.h"

class World
{
public:
    World() = default;
    World(EventBroker* eventBroker) 
        : m_EventBroker(eventBroker)
    { }
    ~World();

    // Create empty entity
    EntityID CreateEntity(EntityID parent = 0);
    // Delete entity and all components within
    void DeleteEntity(EntityID entity);
    // Check if an entity exists
    bool ValidEntity(EntityID entity) const;
    // Register a component type and allocate space for it
    void RegisterComponent(ComponentInfo& ci);
    // Attach a component to an entity and fill it with default values
    ComponentWrapper AttachComponent(EntityID entity, const std::string& componentType);
    // Check if an entity has a component
    bool HasComponent(EntityID entity, const std::string& componentType) const;
    // Get a component of an entity
    ComponentWrapper GetComponent(EntityID entity, const std::string& componentType);
    // Delete a component off an entity
    void DeleteComponent(EntityID entity, const std::string& componentType);
    // Get all components of the specified type
    const ComponentPool* GetComponents(const std::string& componentType);
    // Get entity parent
    EntityID GetParent(EntityID entity);
    // Change the parent of an entity
    void SetParent(EntityID entity, EntityID parent);
    // Get children of an entity
    const std::pair<std::unordered_multimap<EntityID, EntityID>::const_iterator, std::unordered_multimap<EntityID, EntityID>::const_iterator> GetChildren(EntityID entity);
    // Get all component pools
    const std::unordered_map<std::string, ComponentPool*>& GetComponentPools() const { return m_ComponentPools; }
    // Get the entity children map
    const std::unordered_multimap<EntityID, EntityID>& GetEntityChildren() const { return m_EntityChildren; }
    // Set the textual name of an entity
    void SetName(EntityID entity, const std::string& name);
    // Get the textual name of an entity
    std::string GetName(EntityID entity) const;

private:
    EventBroker* m_EventBroker = nullptr;
    EntityID m_CurrentEntityID = 0;

    std::unordered_map<EntityID, EntityID> m_EntityParents;
    // TODO: This should be a more effective structure
    std::unordered_multimap<EntityID, EntityID> m_EntityChildren;
    std::unordered_map<std::string, ComponentPool*> m_ComponentPools;
    std::unordered_map<EntityID, std::string> m_EntityNames;

    EntityID generateEntityID();
    void deleteEntityRecursive(EntityID entity, bool cascaded = false);
};

#endif