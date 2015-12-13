#include "Core/ComponentPool.h"



ComponentWrapper ComponentPoolForwardIterator::operator*() const
{
    char* data = &(*m_MemoryPoolIterator);
    ComponentWrapper wrapper(m_ComponentInfo, data);
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

ComponentPoolForwardIterator& ComponentPoolForwardIterator::operator++(int)
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

//const ::ComponentInfo& ComponentPool::ComponentInfo() const
//{
//    return m_ComponentInfo;
//}

ComponentWrapper ComponentPool::Allocate(EntityID entity)
{
    char* data = m_Pool.Allocate();
    memcpy(data, &entity, sizeof(EntityID));
    m_EntityToComponent[entity] = data;
    return ComponentWrapper(m_ComponentInfo, data);
}

ComponentWrapper ComponentPool::GetByEntity(EntityID ent)
{
    return ComponentWrapper(m_ComponentInfo, m_EntityToComponent.at(ent));
}


bool ComponentPool::KnowsEntity(EntityID ent)
{
    return m_EntityToComponent.find(ent) != m_EntityToComponent.end();
}

void ComponentPool::Delete(ComponentWrapper& wrapper)
{
    m_EntityToComponent.erase(wrapper.EntityID);
    m_Pool.Free(wrapper.Data);
}

ComponentPool::iterator ComponentPool::begin() const
{
    return iterator(m_ComponentInfo, m_Pool.begin(), m_Pool.end());
}

ComponentPool::iterator ComponentPool::end() const
{
    return iterator(m_ComponentInfo, m_Pool.end(), m_Pool.end());
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
