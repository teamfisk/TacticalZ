#ifndef ComponentPool_h__
#define ComponentPool_h__

#include "MemoryPool.h"
#include "ComponentInfo.h"
#include "ComponentWrapper.h"

class ComponentPoolForwardIterator
	: public std::iterator<std::forward_iterator_tag, ComponentWrapper>
{
public:
	ComponentPoolForwardIterator(const ComponentInfo& componentInfo, const MemoryPool<char>::iterator begin, const MemoryPool<char>::iterator end)
		: m_ComponentInfo(componentInfo)
        , m_MemoryPoolIterator(begin)
        , m_MemoryPoolEnd(end)
	{ }

	ComponentPoolForwardIterator(const ComponentPoolForwardIterator& other) = default;
	ComponentPoolForwardIterator(ComponentPoolForwardIterator&& other) = default;
	~ComponentPoolForwardIterator() = default;
	ComponentPoolForwardIterator& operator= (const ComponentPoolForwardIterator& other) = default;

	ComponentPoolForwardIterator& operator++()
	{
        ++m_MemoryPoolIterator;
		return *this;
	}

	ComponentPoolForwardIterator& operator++(int)
	{
		ComponentPoolForwardIterator copyIter(*this);
		operator++();
		return copyIter;
	}

	bool operator!= (const ComponentPoolForwardIterator& other) const
	{
		return m_MemoryPoolIterator != other.m_MemoryPoolIterator;
	}
	
	bool operator== (const ComponentPoolForwardIterator& other) const
	{
		return m_MemoryPoolIterator == other.m_MemoryPoolIterator;
	}

	ComponentWrapper operator* () const
	{
        char* data = &(*m_MemoryPoolIterator);
        ComponentWrapper wrapper(m_ComponentInfo, data);
        return wrapper;
	}

	//ComponentWrapper* operator-> () const
	//{
	//	return &(*m_MemoryPoolIterator);
	//}

private:
    const ComponentInfo& m_ComponentInfo;
	MemoryPool<char>::iterator m_MemoryPoolIterator;
	const MemoryPool<char>::iterator m_MemoryPoolEnd;
};

class ComponentPool
{
public:
	typedef ComponentPoolForwardIterator iterator;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
    typedef ComponentWrapper value_type;
	typedef ComponentWrapper* pointer;
	typedef ComponentWrapper& reference;

    ComponentPool(const ComponentInfo& ci)
        : m_ComponentInfo(ci)
        , m_Pool(ci.Meta.Allocation, ci.Meta.Stride)
    { }

	ComponentPool(const ComponentPool& other) = delete;
	ComponentPool(const ComponentPool&& other) = delete;

    ComponentWrapper New()
    {
        char* data = m_Pool.Allocate();
        return ComponentWrapper(m_ComponentInfo, data);
    }

    void Delete(ComponentWrapper& component)
    {
        m_Pool.Free(component.Data);
    }

	iterator begin() const
	{
		return iterator(m_ComponentInfo, m_Pool.begin(), m_Pool.end());
	}

	iterator end() const
	{
		return iterator(m_ComponentInfo, m_Pool.end(), m_Pool.end());
	}

	//Dumps information about what the pool memory looks like right now 
	//into an output stream (e.g. file/std::cout, anything that has an operator<<)
	//Interpret the data in the memory as InterpretType.
	template <typename InterpretType = char, typename OutStream>
	void Dump(OutStream& out) const
	{
		m_Pool.Dump<InterpretType>(out);
	}

	//Dumps information about what the pool memory looks like right now 
	//into std::cout. Interpret the data in the memory as InterpretType.
	template <typename InterpretType = char>
	void Dump() const
	{
		m_Pool.Dump<InterpretType>();
	}

private:
    const ComponentInfo m_ComponentInfo;
    MemoryPool<char> m_Pool;
};

#endif