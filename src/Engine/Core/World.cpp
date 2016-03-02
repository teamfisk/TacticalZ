#include "Core/World.h"
#include "Core/EEntityDeleted.h"
#include "Core/EComponentDeleted.h"

World::~World()
{
    for (auto& pool : m_ComponentPools) {
        delete pool.second;
    }
}

EntityID World::CreateEntity(EntityID parent /*= 0*/)
{
    EntityID newEntity = generateEntityID();
    if (newEntity == parent) {
        LOG_WARNING("Invalid parent #%i of Entity#%i", newEntity, parent);
        parent = EntityID_Invalid;
    }
    m_EntityParents[newEntity] = parent;
    m_EntityChildren.insert(std::make_pair(parent, newEntity));
    return newEntity;
}

void World::DeleteEntity(EntityID entity)
{
    deleteEntityRecursive(entity, false);
}

bool World::ValidEntity(EntityID entity) const
{
    return m_EntityParents.find(entity) != m_EntityParents.end();
}

void World::RegisterComponent(ComponentInfo& ci)
{
    if (m_ComponentPools.find(ci.Name) == m_ComponentPools.end()) {
        m_ComponentPools[ci.Name] = new ComponentPool(ci);
    }
}

ComponentWrapper World::AttachComponent(EntityID entity, const std::string& componentType)
{
    // TODO: Allocate dynamic pool if component isn't registered
    ComponentPool* pool = m_ComponentPools.at(componentType);
    const ComponentInfo& ci = pool->ComponentInfo();

    // Allocate space for the component
    ComponentWrapper c = pool->Allocate(entity);
    // Write default values
    memcpy(c.Data, ci.Defaults.get(), ci.Stride);

    return c;
}

bool World::HasComponent(EntityID entity, const std::string& componentType) const
{
    auto it = m_ComponentPools.find(componentType);
    if (it == m_ComponentPools.end()) {
        return false;
    } else {
        return it->second->KnowsEntity(entity);
    }
}

ComponentWrapper World::GetComponent(EntityID entity, const std::string& componentType)
{
    ComponentPool* pool = m_ComponentPools.at(componentType);
    return pool->GetByEntity(entity);
}

void World::DeleteComponent(EntityID entity, const std::string& componentType)
{
    ComponentPool* pool = m_ComponentPools.at(componentType);
    ComponentWrapper c = pool->GetByEntity(entity);
    pool->Delete(c);

    if (m_EventBroker != nullptr) {
        Events::ComponentDeleted e;
        e.Entity = entity;
        e.ComponentType = componentType;
        e.Cascaded = false;
        m_EventBroker->Publish(e);
    }
}

const ComponentPool* World::GetComponents(const std::string& componentType)
{
    auto it = m_ComponentPools.find(componentType);
    return (it != m_ComponentPools.end()) ? it->second : nullptr;
}

EntityID World::GetParent(EntityID entity)
{
    return m_EntityParents.at(entity);
}

void World::SetParent(EntityID entity, EntityID parent)
{
    // Don't allow an entity to be a child to itself!
    if (entity == parent) {
        // HACK: We purposely don't check the whole hierarchy of children here, since it would be way too slow.
        // This might result in infinite loops if an entity somehow ends up as a child.
        return;
    }

    EntityID lastParent = m_EntityParents.at(entity);
    auto parentChildren = m_EntityChildren.equal_range(lastParent);
    for (auto it = parentChildren.first; it != parentChildren.second; it++) {
        if (it->second == entity) {
            m_EntityChildren.erase(it);
            break;
        }
    }

    m_EntityParents[entity] = parent;
    m_EntityChildren.insert(std::make_pair(parent, entity));
}

const std::pair<std::unordered_multimap<EntityID, EntityID>::const_iterator, std::unordered_multimap<EntityID, EntityID>::const_iterator> World::GetChildren(EntityID entity)
{
    return m_EntityChildren.equal_range(entity);
}

void World::SetName(EntityID entity, const std::string& name)
{
    m_EntityNames[entity] = name;
}

std::string World::GetName(EntityID entity) const
{
    if (entity == EntityID_Invalid) {
        return "EntityID_Invalid";
    }

    auto it = m_EntityNames.find(entity);
    if (it != m_EntityNames.end()) {
        return it->second;
    } else {
        return std::string();
    }
}

EntityID World::generateEntityID()
{
    // TODO: Make EntityID generation smarter
    return m_CurrentEntityID++;
}

void World::deleteEntityRecursive(EntityID entity, bool cascaded /*= false*/)
{
    // Don't attempt to delete entities that don't exist anyway
    if (!ValidEntity(entity)) {
        return;
    }

    if (m_EventBroker != nullptr) {
        Events::EntityDeleted e;
        e.DeletedEntity = entity;
        e.Cascaded = cascaded;
        m_EventBroker->Publish(e);
    }

    // Delete components
    for (auto& pair : m_ComponentPools) {
        auto& pool = pair.second;
        if (pool->KnowsEntity(entity)) {
            auto& c = pool->GetByEntity(entity);
            pool->Delete(c);
            if (m_EventBroker != nullptr) {
                Events::ComponentDeleted e;
                e.Entity = entity;
                e.ComponentType = pair.first;
                e.Cascaded = true;
                m_EventBroker->Publish(e);
            }
        }
    }

    // Loop through children
    std::vector<EntityID> childrenToDelete;
    auto children = m_EntityChildren.equal_range(entity);
    for (auto it = children.first; it != children.second; ++it) {
        childrenToDelete.push_back(it->second);
    }
    for (auto& child : childrenToDelete) {
        deleteEntityRecursive(child, true);
    }

    EntityID parent = m_EntityParents.at(entity);
    m_EntityParents.erase(entity);
    auto parentChildren = m_EntityChildren.equal_range(parent);
    for (auto it = parentChildren.first; it != parentChildren.second; ++it) {
        if (it->second == entity) {
            m_EntityChildren.erase(it);
            break;
        }
    }

    // Erase potential name
    m_EntityNames.erase(entity);
}

