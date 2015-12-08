#include "Core/World.h"

World::~World()
{
    for (auto& pool : m_ComponentPools) {
        delete pool.second;
    }
}

EntityID World::CreateEntity(EntityID parent /*= 0*/)
{
    EntityID newEntity = generateEntityID();
    m_EntityParents[newEntity] = parent;
    if (parent != 0) {
        m_EntityChildren.insert(std::make_pair(parent, newEntity));
    }
    return newEntity;
}

void World::RegisterComponent(ComponentInfo& ci)
{
    m_ComponentPools[ci.Name] = new ComponentPool(ci);
}

ComponentWrapper World::AttachComponent(EntityID entity, std::string componentType)
{
    ComponentPool* pool = m_ComponentPools.at(componentType);
    const ComponentInfo& ci = pool->ComponentInfo();

    // Allocate space for the component
    ComponentWrapper c = pool->Allocate(entity);
    // TODO: Write default values
    if (ci.Defaults != nullptr) {
        //memcpy(c.Data, ci.Defaults.get(), ci.Meta.Stride);
    }

    return c;
}

ComponentWrapper World::GetComponent(EntityID entity, std::string componentType)
{
    ComponentPool* pool = m_ComponentPools.at(componentType);
    return pool->GetByEntity(entity);
}

const ComponentPool& World::GetComponents(std::string componentType)
{
    return *m_ComponentPools.at(componentType);
}

EntityID World::generateEntityID()
{
    // TODO: Make EntityID generation smarter
    return m_CurrentEntityID++;
}

