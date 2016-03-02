#ifndef MemoryPool_h__
#define MemoryPool_h__
#include "Common.h"

template <typename T>
class MemoryPoolForwardIterator;

namespace DisableMemoryPool
{
//if true -> Pool allocation is not used when calling Allocate/Free, just use regular dynamic allocation. 
//if false -> Use pool allocation.
//Should default to false, unless the DisableMemoryPool is true in the Config.ini files.
extern bool Value;
}

//This is the class to use if you want to allocate blocks (slots) of raw memory, with a fixed maximum size (stride).
//Additionally, if you know that every memory-block will contain one object of a specific type, (i.e. the stride for the slot 
//will the size of the object type) you should use ObjectPool<T> instead, your life will become easier.
//The memory returned by pool.Allocate() has has no type-information
//Works similarly to malloc(), but it allocates a number of slots of a certain size.
//Basically, pool.Allocate() does the same as malloc(m_Stride).
//
//When the pool goes out of scope (when it is destructed) it will free all memory that was allocated automatically.
//
//The template input type T does not affect the structure of the memory pool, it will 
//only affect how the iterators access the pool data, for example: 
//MemoryPool<int> p; MemoryPool<int>::iterator iter = p.begin();
//Here, *iter will return the data in the first allocated slot in the pool, interpreted as an int.
//If you only intend to use the pointers returned by Allocate(), and not iterate through the pool, 
//T is never used and can safely be set to anything. e.g. char.
template <typename T>
class MemoryPool
{
	template <typename T>
	friend class MemoryPoolForwardIterator;
public:
	typedef MemoryPoolForwardIterator<T> iterator;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;

	MemoryPool() 
		: m_StartAddress(nullptr)
		, m_SlotIsAllocated()
		, m_NumSlots(0)
		, m_Stride(0)
		, m_NumAllocatedSlots(0)
		, m_LowestAllocatedSlot(0)
	{ }

	//[stride] is the number of bytes allocated per slot in the pool, 
	//the smallest amount of memory you can get from Allocate(). (Usually, the maximum size of an object to allocate.)
	//[numMaxElements] is the number of slots in the pool. That is, how many stride-sized memory blocks 
	//the pool will have. (Usually, how many objects can be allocated.)
	//These parameters are used to preallocate memory for efficency.
	//If numMaxElements is exceeded, the pool will start allocate memory dynamically, "outside the pool", and is much slower.
	MemoryPool(size_t numMaxElements, size_t stride)
		: m_StartAddress(new char[numMaxElements*stride])
		, m_SlotIsAllocated(numMaxElements, false)
		, m_NumSlots(numMaxElements)
		, m_Stride(stride)
		, m_NumAllocatedSlots(0)
		, m_CurrentAllocSlot(0)
		, m_LowestAllocatedSlot(m_NumSlots)
	{ }

    MemoryPool(const MemoryPool<T>& other)
		: m_StartAddress(new char[other.m_NumSlots*other.m_Stride])
		, m_SlotIsAllocated(other.m_SlotIsAllocated)
        , m_ExtraMemory()
		, m_NumSlots(other.m_NumSlots)
		, m_LowestAllocatedSlot(other.m_LowestAllocatedSlot)
		, m_NumAllocatedSlots(other.m_NumAllocatedSlots)
		, m_Stride(other.m_Stride)
		, m_CurrentAllocSlot(other.m_CurrentAllocSlot)
    {
        // Copy statically allocated pool
        memcpy(m_StartAddress, other.m_StartAddress, m_NumSlots*m_Stride);
        // Copy dynamically allocated memory
        for (char* otherAddr : other.m_ExtraMemory) {
            char* addr = (char*)malloc(m_Stride);
            memcpy(addr, otherAddr, m_Stride);
            m_ExtraMemory.push_back(addr);
        }
    }
	MemoryPool(const MemoryPool<T>&& other) = delete;

	//Free all memory that has been allocated.
	~MemoryPool()
	{
		if (m_StartAddress != nullptr) {
			delete[] m_StartAddress;
			m_StartAddress = nullptr;
		}
        for (char* addr : m_ExtraMemory) {
            free(addr);
        }
		m_ExtraMemory.clear();
	}

	//Allocates space for a contingous block of memory the size of [stride] bytes, and returns a pointer to the data.
	//The data is not touched, and remains uninitialized, i.e. it will have random values in it.
	//If element cannot be allocated in the pool, because the memory ran out, memory is allocated dynamically with malloc() "outside the pool".
	char* Allocate()
	{
		for (; m_CurrentAllocSlot < m_NumSlots && m_SlotIsAllocated[m_CurrentAllocSlot] && !DisableMemoryPool::Value; ++m_CurrentAllocSlot);
		if (m_CurrentAllocSlot < m_NumSlots && !DisableMemoryPool::Value) {
			if (m_LowestAllocatedSlot > m_CurrentAllocSlot)
				m_LowestAllocatedSlot = m_CurrentAllocSlot;
			//Mark the slot as allocated.
			m_SlotIsAllocated[m_CurrentAllocSlot] = true;
			++m_NumAllocatedSlots;
			//Also increment slot to allocate.
			return m_StartAddress + m_Stride*m_CurrentAllocSlot++;
		}
		else {
			m_ExtraMemory.push_back((char*)malloc(m_Stride));
			//We should preferably not enter here to avoid performance issues. Set more numMaxElements in constructor instead.
            if (!DisableMemoryPool::Value) {
			    LOG_DEBUG("Allocated slots exceed Pool size, extra memory allocated dynamically. Pool size: %u, dynamic size: %u.", m_NumSlots, m_ExtraMemory.size());
            }
			return m_ExtraMemory.back();
		}
	}

	//Free memory that was allocated earlier with Allocate().
	void Free(char* obj)
	{
		//If memory was allocated in the memory pool.
		//We are guaranteed to enter here if we have zero malloc() allocations, 
		//or if the obj was allocated in the pool,
		//however, comparisons is technically undefined 
		//(i.e. IsAllocatedInPool may give false positives)
		//if it was malloc():ed 
		//so, we may enter here even if we shouldn't.
		if (!DisableMemoryPool::Value && IsAllocatedInPool(obj)) {
			--m_NumAllocatedSlots;
			const size_t freeSlot = (obj - m_StartAddress) / m_Stride;
			m_SlotIsAllocated[freeSlot] = false;
			if (freeSlot < m_CurrentAllocSlot)
				m_CurrentAllocSlot = freeSlot;
			//If we happened to remove the begin() slot, find the next one.
			//Increment until we find an allocated slot, or go out of bounds.
			while (m_LowestAllocatedSlot != m_NumSlots && !m_SlotIsAllocated[m_LowestAllocatedSlot])
				++m_LowestAllocatedSlot;
		}
		//If memory was allocated dynamically with malloc because we didn't have enough storage in pool.
		//I.e: if numMallocs > 0 and obj is outside [m_StartAddress-->m_NumSlots].
		else {
			m_ExtraMemory.erase(find(m_ExtraMemory.begin(), m_ExtraMemory.end(), obj));
			free(obj);
		}
	}

	//Returns an iterator pointing to the first element.
	//If pool is empty it will be equal to end() and shall not be dereferenced.
	iterator begin() const
	{
		//If pool is empty, lowest slot will be numSlots, 
		//and m_ExtraMemory.size will be 0, so begin == end.
		return iterator(this, m_LowestAllocatedSlot);
	}

	//Returns an iterator pointing beyond the last element.
	//This shall not be dereferenced (Gives a run-time error).
	iterator end() const
	{
		return iterator(this, m_NumSlots + m_ExtraMemory.size());
	}

	//Returns true iff the pool has no allocated elements.
	bool empty() const
	{
		return m_LowestAllocatedSlot == m_NumSlots && m_ExtraMemory.empty();
	}

	//Returns the total number of allocated elements.
	size_t size() const
	{
		return m_NumAllocatedSlots + m_ExtraMemory.size();
	}

	//Returns the number of elements allocated inside the pool boundary.
	size_t PoolSize() const
	{
		return m_NumAllocatedSlots;
	}

	//Returns the number of elements allocated outside the pool boundary (in dynamic extra space).
	size_t ExtraSize() const
	{
		return m_ExtraMemory.size();
	}

	//Dumps information about what the pool memory looks like right now 
	//into an output stream (e.g. file/std::cout, anything that has an operator<<)
	//Interpret the data in the memory as InterpretType.
	template <typename InterpretType = char, typename OutStream>
	void Dump(OutStream& out) const
	{
		out << "Primary pool memory: Allocated Slots=" << m_NumAllocatedSlots << std::endl;
		for (size_t i = 0; i < m_NumSlots; ++i) {
			out << "Slot nr " << i << ": ";
			for (size_t c = 0; c < m_Stride/sizeof(InterpretType); ++c)
				out << (*reinterpret_cast<InterpretType*>(m_StartAddress + i*m_Stride + c)) << "\t";
			out << "Allocated = " << m_SlotIsAllocated[i] << std::endl;
		}
		out << "Dynamic extra pool memory: Slots=" << m_ExtraMemory.size() << std::endl;
		for (size_t i = 0; i < m_ExtraMemory.size(); ++i) {
			out << "Extra " << i << ": ";
			for (size_t c = 0; c < m_Stride / sizeof(InterpretType); ++c)
				out << (*reinterpret_cast<InterpretType*>(m_ExtraMemory[i] + c)) << "\t";
			out << std::endl;
		}
	}

	//Dumps information about what the pool memory looks like right now 
	//into std::cout. Interpret the data in the memory as InterpretType.
	template <typename InterpretType = char>
	void Dump() const
	{
		Dump<InterpretType>(std::cout);
	}

private:
	char* m_StartAddress;
	std::vector<bool> m_SlotIsAllocated;
	std::vector<char*> m_ExtraMemory;
	size_t m_NumSlots;
	size_t m_LowestAllocatedSlot;
	size_t m_NumAllocatedSlots;
	size_t m_Stride;
	size_t m_CurrentAllocSlot;
	bool IsAllocatedInPool(char* p)
	{
		//If we went outside the pool limits and used dynamic allocation.
		if (!m_ExtraMemory.empty())
		{
			//Assumes that std::uintptr_t is a thing here. No idea what linux does here.
#ifdef UINTPTR_MAX
			//Assumes the memory has a linear address space, no weird jumps. Should work on modern platforms.
			//Assumes unsigned values loop back to very positive when they go negative. Standard C++ behavior.
			return (reinterpret_cast<std::uintptr_t>(p) - reinterpret_cast<std::uintptr_t>(m_StartAddress) < m_NumSlots * m_Stride);
#else
			//Fallback on inefficient loop otherwise.
			for (size_t i = 0; i < m_NumSlots; i++)
				if (p == m_StartAddress + i * m_Stride)
					return true;
			return false;
#endif
		}
		return true;	//If we don't go outside the pool boundary, this will always be true.
	}
};

template <typename T>
class MemoryPoolForwardIterator
	: public std::iterator<std::forward_iterator_tag, T>
{
public:
	MemoryPoolForwardIterator(const MemoryPool<T>* pool, size_t slotPos)
		: m_Pool(pool)
		, m_Pos(slotPos)
	{ }

	MemoryPoolForwardIterator(const MemoryPoolForwardIterator<T>& other) = default;
	MemoryPoolForwardIterator(MemoryPoolForwardIterator<T>&& other) = default;
	~MemoryPoolForwardIterator() = default;
	MemoryPoolForwardIterator<T>& operator= (const MemoryPoolForwardIterator<T>& other) = default;

	//Prefix increment i.e. ++iter. More efficient than post increment.
	MemoryPoolForwardIterator& operator++()
	{
		//Increment position, if pos is in the pool area and the slot isn't allocated, 
		//keep checking the next position.
		while (++m_Pos < m_Pool->m_NumSlots && !m_Pool->m_SlotIsAllocated[m_Pos]);
		return *this;
	}

	//Postfix increment i.e. iter++. Prefer pre-increment (++iter) for efficiency.
	MemoryPoolForwardIterator operator++(int)
	{
		MemoryPoolForwardIterator<T> copyIter(*this);
		operator++();
		return copyIter;
	}

	bool operator!= (const MemoryPoolForwardIterator& other) const
	{
		return m_Pos != other.m_Pos;
	}
	
	bool operator== (const MemoryPoolForwardIterator& other) const
	{
		return m_Pos == other.m_Pos;
	}

	T& operator* () const
	{
		return *this->operator->();
	}

	T* operator-> () const
	{
		//If pos < numSlots then the iterator is in the pool.
		//Else it is in the extra memory.

		//NOTE: If you get a "vector subscript out of range" error here, 
		//then you possibly dereferenced the end() iterator (don't do that).
		return (m_Pos < m_Pool->m_NumSlots)
			? (reinterpret_cast<T*>(m_Pool->m_StartAddress + m_Pool->m_Stride*m_Pos))
			: (reinterpret_cast<T*>(m_Pool->m_ExtraMemory[m_Pos - m_Pool->m_NumSlots]));
	}

private:
	const MemoryPool<T>* m_Pool;
	size_t m_Pos;
};

#endif