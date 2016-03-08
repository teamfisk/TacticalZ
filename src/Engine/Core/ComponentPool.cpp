#include "Core/ComponentPool.h"

ComponentPoolForwardIterator::ComponentPoolForwardIterator(ComponentPool* pool, const MemoryPool<char>::iterator begin, const MemoryPool<char>::iterator end)
    : m_ComponentPool(pool)
    , m_ComponentInfo(pool->m_ComponentInfo)
    , m_MemoryPoolIterator(begin)
    , m_MemoryPoolEnd(end)
{ }

ComponentWrapper ComponentPoolForwardIterator::operator*() const
{
    char* data = &(*m_MemoryPoolIterator);
    EntityID entity = *reinterpret_cast<EntityID*>(data);
    ComponentWrapper wrapper(m_ComponentInfo, data, &m_ComponentPool->m_DirtySet[entity]);
    return wrapper;
}

bool ComponentPoolForwardIterator::operator==(const ComponentPoolForwardIterator& other) const
{
    return m_MemoryPoolIterator == other.m_MemoryPoolIterator;
}

bool ComponentPoolForwardIterator::operator!=(const ComponentPoolForwardIterator& other) const
{
    return m_MemoryPoolIterator != other.m_MemoryPoolIterator;
}

ComponentPoolForwardIterator ComponentPoolForwardIterator::operator++(int)
{
    ComponentPoolForwardIterator copyIter(*this);
    operator++();
    return copyIter;
}

ComponentPoolForwardIterator& ComponentPoolForwardIterator::operator++()
{
    ++m_MemoryPoolIterator;
    return *this;
}

ComponentPool::ComponentPool(const ComponentPool& other)
    : m_ComponentInfo(other.m_ComponentInfo)
    , m_Pool(other.m_Pool)
    , m_EntityToComponent()
{
    // Update EntityToComponent pointers
    for (char& ptr : m_Pool) {
        EntityID entity = *reinterpret_cast<EntityID*>(&ptr);
        m_EntityToComponent[entity] = &ptr;
    }

    // Duplicate strings
    for (auto& name : m_ComponentInfo.StringFields) {
        for (auto& c : *this) {
            std::string& val = c[name];
            ComponentWrapper::SolidifyStrings(c);
        }
    }
}

ComponentPool::~ComponentPool()
{
    // Destroy component data
    for (auto& c : *this) {
        ComponentWrapper::Destroy(c.Info, c.Data);
    }
}

//const ::ComponentInfo& ComponentPool::ComponentInfo() const
//{
//    return m_ComponentInfo;
//}

ComponentWrapper ComponentPool::Allocate(EntityID entity)
{
    // Allocate pool data
    char* data = m_Pool.Allocate();
    // Copy EntityID
    memcpy(data, &entity, sizeof(EntityID));

    m_EntityToComponent[entity] = data;
    ComponentWrapper component(m_ComponentInfo, data, &m_DirtySet[entity]);

    // Copy defaults
    memcpy(component.Data, m_ComponentInfo.Defaults.get(), m_ComponentInfo.Stride);
    ComponentWrapper::SolidifyStrings(component);

    return component;
}

ComponentWrapper ComponentPool::GetByEntity(EntityID ent)
{
    auto data = m_EntityToComponent.at(ent);
    auto bitField = &m_DirtySet[ent];
    return ComponentWrapper(m_ComponentInfo, data, bitField);
}

bool ComponentPool::KnowsEntity(EntityID ent)
{
    return m_EntityToComponent.find(ent) != m_EntityToComponent.end();
}

void ComponentPool::Delete(ComponentWrapper& wrapper)
{
    ComponentWrapper::Destroy(wrapper.Info, wrapper.Data);
    m_EntityToComponent.erase(wrapper.EntityID);
    m_Pool.Free(wrapper.Data - sizeof(EntityID));
    m_DirtySet.erase(wrapper.EntityID);
}

ComponentPool::iterator ComponentPool::begin()
{
    return iterator(this, m_Pool.begin(), m_Pool.end());
}

ComponentPool::iterator ComponentPool::end()
{
    return iterator(this, m_Pool.end(), m_Pool.end());
}

size_t ComponentPool::size() const
{
    return m_Pool.size();
}

template <typename InterpretType /*= char*/>
void ComponentPool::Dump() const
{
    m_Pool.Dump<InterpretType>();
}

template <typename InterpretType /*= char*/, typename OutStream>
void ComponentPool::Dump(OutStream& out) const
{
    m_Pool.Dump<InterpretType>(out);
}
