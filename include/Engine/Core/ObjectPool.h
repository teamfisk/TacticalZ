#ifndef ObjectPool_h__
#define ObjectPool_h__

#include "MemoryPool.h"

// - Example usage code below class definition.
//
//This is the class to use if you want to dynamically allocate memory to hold a specific object type.
//
//When the pool goes out of scope (when it is destructed) it destruct all allocated objects and free all memory that was allocated in the pool.
template <typename T>
class ObjectPool
{
public:
	typedef MemoryPoolForwardIterator<T> iterator;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;

	ObjectPool()
		: m_Pool()
	{ }

	//[numMaxElements] is the number of objects in the pool. This parameter is used to preallocate memory.
	//If numMaxElements is exceeded, the pool will start allocate memory dynamically, "outside the pool", which is much slower.
	ObjectPool(size_t numMaxElements)
		: m_Pool(numMaxElements, sizeof(T))
	{ }

	//Destruct all elements in the pool that are still allocated.
	//Lastly, m_Pool will go out of scope and destruct, freeing all the memory.
	~ObjectPool()
	{
		for (T &o : *this)
			o.~T();
	}

	//Allocates memory to hold one T object, constructs an object 
	//by supplying any arguments to T(...) constructor.
	//Then returns a pointer to its memory.
	//
	//Works similiarly to new below:
	//T* objPointer = new T(...);	//<--
	// ...
	//delete objPointer;
	template<typename... Arguments>
	T* New(Arguments... args)
	{
		return new (m_Pool.Allocate()) T(args...);
	}

	//Calls the destructor of the object pointed to by input parameter.
	//Then frees the memory. 
	//Works similiarly to delete below:
	//T* objPointer = new T(...);
	// ...
	//delete objPointer;			//<--
	void Delete(T* pObject)
	{
		pObject->~T();
		m_Pool.Free(reinterpret_cast<char*>(pObject));
	}

	//Calls the destructor of the object pointed to by iterator.
	//Then frees the memory. 
	void Delete(iterator objIterator)
	{
		pool.Delete(&(*objIterator));
	}

	//Returns an iterator pointing to the first element.
	//If pool is empty it will be equal to end() and shall not be dereferenced.
	iterator begin() const
	{
		return m_Pool.begin();
	}

	//Returns an iterator pointing beyond the last element.
	//This shall not be dereferenced (Gives a run-time error).
	iterator end() const
	{
		return m_Pool.end();
	}

	//Returns true iff the pool has no allocated elements.
	bool empty() const
	{
		return m_Pool.empty();
	}

	//Returns the total number of allocated elements.
	size_t size() const
	{
		return m_Pool.size();
	}

	//Returns the number of allocated elements but not from dynamically allocated extra space.
	size_t PoolSize() const
	{
		return m_Pool.PoolSize();
	}

	//Returns the number of elements allocated outside the pool boundary (in dynamic extra space).
	size_t ExtraSize() const
	{
		return m_Pool.ExtraSize();
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
	MemoryPool<T> m_Pool;
};


//------------Simple use case example code scenario-----------------
//
//		//User defined type that the pool will hold.
//		struct HappyStruct {
//			int i;
//			float f;
//			HappyStruct() = default;
//			HappyStruct(int ii, float ff) : i(ii), f(ff) { }
//			bool operator == (const HappyStruct& o)		//std::find wants a == to search through the pool.
//			{
//				return o.i == i && o.f == f;
//			}
//		};
//
//		//The pool can hold 100 HappyStruct's, before it starts to struggle and we get performance issues.
//		ObjectPool<HappyStruct> pool(100);
//
//		//For some reason, we want to allocate a struct dynamically.
//		//Usually, you would do this:		HappyStruct* s = new HappyStruct(17, 2.4142f);
//		//Instead:
//		HappyStruct* pHappy = pool.New(17, 2.4142f);		
//
//		//If you want to be able to free the memory manually (e.g. for temporary objects), 
//		//to free space for other objects, save the return value in pHappy and
//		//when it is not needed anymore release it back to the pool.
//		//Usually, you would do this:		delete pHappy;
//		//Instead:
//		pool.Delete(pHappy);
//
//		//Objects that are not deleted manually will be destructed and deallocated automatically when the pool 
//		//destructs, so elements can be added to the pool like this.
//		pool.New(1, 0.0f);
//		pool.New(2, 0.0f);
//		pool.New(3, 0.0f);
//		pool.New(5, 0.0f);
//		pool.New(4, 0.0f);
//		
//		//You can use some standard functions that operate on iterators,
//		//for example if you really want to destroy the element with a 5. (Less efficient (it loops) than deleting the pointer directly, as above)
//		ObjectPool<S>::iterator it = find(pool.begin(), pool.end(), S(5, 0));	//Find the first element that is equal to S(5,0)
//		pool.Delete(it);
//
//		//You can iterate through all allocated elements in the pool, like you would through a collection such as vector:
//		//Version with awesome C++11 range-based syntax:
//		for (auto& element : pool)
//			element.i += 2;
//		//Version with standard looping:
//		for (auto& iter = pool.begin(); iter != pool.end(); ++iter)
//			iter->i += 2;
//
//		//Output a snapshot of the memory in the pool, with the memory data interpreted as int, to a stream.
//		std::ostream& out = std::cout;
//		pool.Dump<int>(out);
//		//Output a snapshot of the memory in the pool, with the memory data interpreted as char (bytedata), to a file.
//		std::ofstream file("randomFile.txt");
//		pool.Dump(file);
//		file.close();
//

#endif