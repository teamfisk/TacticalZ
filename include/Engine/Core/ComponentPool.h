#ifndef ComponentPool_h__
#define ComponentPool_h__

#include <set>
#include "MemoryPool.h"
#include "ComponentInfo.h"
#include "ComponentWrapper.h"
#include "DirtySet.h"


class ComponentPool;
class ComponentPoolForwardIterator
	: public std::iterator<std::forward_iterator_tag, ComponentWrapper>
{
public:
    ComponentPoolForwardIterator(ComponentPool* pool, const MemoryPool<char>::iterator begin, const MemoryPool<char>::iterator end);

	ComponentPoolForwardIterator(const ComponentPoolForwardIterator& other) = default;
	ComponentPoolForwardIterator(ComponentPoolForwardIterator&& other) = default;
	~ComponentPoolForwardIterator() = default;
	ComponentPoolForwardIterator& operator=(const ComponentPoolForwardIterator& other) = default;
	ComponentPoolForwardIterator& operator++();
	ComponentPoolForwardIterator operator++(int);
	bool operator!=(const ComponentPoolForwardIterator& other) const;
	bool operator==(const ComponentPoolForwardIterator& other) const;
	ComponentWrapper operator*() const;

private:
    ComponentPool* m_ComponentPool;
    const ComponentInfo& m_ComponentInfo;
	MemoryPool<char>::iterator m_MemoryPoolIterator;
	const MemoryPool<char>::iterator m_MemoryPoolEnd;
};

class ComponentPool
{
    friend class ComponentPoolForwardIterator;
public:
	typedef ComponentPoolForwardIterator iterator;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
    typedef ComponentWrapper value_type;
	typedef ComponentWrapper* pointer;
	typedef ComponentWrapper& reference;

    ComponentPool(const ::ComponentInfo& ci) 
        : m_ComponentInfo(ci)
        , m_Pool(ci.Meta->Allocation, ci.GetHeaderSize() + ci.Stride)
    { }
    ~ComponentPool();
	ComponentPool(const ComponentPool& other);
	ComponentPool(const ComponentPool&& other) = delete;

    const ::ComponentInfo& ComponentInfo() const { return m_ComponentInfo; }

    // Allocate space for a component and store which entity it belongs to in internal structure
    ComponentWrapper Allocate(EntityID entity);
    // Get the component belonging to a specific entity
    ComponentWrapper GetByEntity(EntityID ent);
    // Returns true if the pool contains a component for the specified entity
    bool KnowsEntity(EntityID ent);
    // Delete a component and free its memory
    void Delete(ComponentWrapper& wrapper);

    iterator begin();
	iterator end();
    size_t size() const;

	//Dumps information about what the pool memory looks like right now 
	//into an output stream (e.g. file/std::cout, anything that has an operator<<)
	//Interpret the data in the memory as InterpretType.
	template <typename InterpretType = char, typename OutStream>
	void Dump(OutStream& out) const;

	//Dumps information about what the pool memory looks like right now 
	//into std::cout. Interpret the data in the memory as InterpretType.
	template <typename InterpretType = char>
	void Dump() const;

private:
    ::ComponentInfo m_ComponentInfo;
    MemoryPool<char> m_Pool;
    std::unordered_map<EntityID, char*> m_EntityToComponent;
    DirtySet m_DirtySet;
};

#endif