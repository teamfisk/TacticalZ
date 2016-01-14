#ifndef ResourceManager_h__
#define ResourceManager_h__

#include <functional>
#include <set>
#include <fstream>

#include "../Common.h"
#include "Util/UnorderedMapPair.h"
#include "Util/FileWatcher.h"

/** Base Resource class.

	Implement this class for every resource to be handled by the resource manager.
*/
class Resource
{
	friend class ResourceManager;

protected:
	Resource() { }

public:
	// Pretend that this is a pure virtual function that you have to implement
	// FIXME: Why did we do this again instead of just using the constructor?
	// static Resource* Create(std::string resourceName);

	virtual void Reload() {	}
	virtual void OnChildReloaded(Resource* child) { }

	unsigned int TypeID;
	unsigned int ResourceID;
};

//Any class inheriting from this class will always be loaded on the master thread, not on a parallel worker thread.
//This is important in case some instructions must be executed on the main thread, e.g. OpenGL commands, like glBindBuffer.
//This resource can still be loaded asyncronously, but it will not be loaded in a thread, instead it's constructor will 
//be called once on every ResourceManager::Load, just throw StillLoadingException in the constructor if it is not done yet.
class ThreadUnsafeResource : public Resource
{
    friend class ResourceManager;
protected:
    //Should be thrown in a Resource's constructor if it cannot complete because another resource is still loading.
    //Not actually an error, just a message to the ResourceManager.
    struct StillLoadingException : public std::exception
    {
        virtual const char* what() const throw()
        {
            return "Resource is still loading.";
        }
    };
};

/** Singleton resource manager to keep track of and cache any external engine assets */
class ResourceManager
{
private:
	ResourceManager();

public:
	/*static ResourceManager& Instance()
	{
	static ResourceManager s;
	return s;
	}*/

	template <typename T>
	static void RegisterType(std::string typeName);

	/** Checks if a resource is in cache

		@param resourceType Resource type as string.
		@param resourceName Fully qualified name of the resource to preload.
	*/
	// TODO: Templateify
	static bool IsResourceLoaded(std::string resourceType, std::string resourceName);
    
	/** If Async is false: Hot-loads a resource and caches it for future use. 
        Fairly safe to assume that return value is always a valid pointer, will only return nullptr on error.
        
        If Async is true: If the resource is not loaded yet, starts loading the resource 
        in the background and returns nullptr immediately.
        If the resource has been loaded already, return a pointer to it.

		@tparam T Resource type.
		@tparam Async Set this to true if the resource should be loaded asyncronously.
		@param resourceName Fully qualified name of the resource to load.
	*/
    template <typename T, bool Async = false>
    static T* Load(std::string resourceName, Resource* parent = nullptr);

	/** Reloads an already loaded resource, keeping its resource ID intact.

	 	@tparam T Resource type.
	 	@param resourceName Fully qualified name of the resource to reload.
	*/
    static void Reload(std::string resourceName);
    
    static void Release(std::string resourceType, std::string resourceName);

	static void Update();

private:
    //Represents a pointer value to signify that a resource haven't failed, but is not fully loaded.
    class SpecialResourcePointer
    {
    public:
        SpecialResourcePointer()
            : m_Val(new Resource())
        {}
        ~SpecialResourcePointer()
        {
            delete m_Val;
        }
        //Make this class implicitly convertible to the Resource*.
        operator Resource* const() const { return m_Val; }
    private:
        Resource* const m_Val;
    };
    //This is a haxy way to make sure that IsMainThread is run when the program starts, so the master thread id is set.
    struct MasterThreadChecker
    {
        MasterThreadChecker()
        {
            ResourceManager::IsMainThread();
        }
    };
    const static SpecialResourcePointer m_StillLoading;
    const static MasterThreadChecker m_Checker;

	static std::unordered_map<std::string, std::string> m_CompilerTypenameToResourceType;
    static std::unordered_map<std::string, std::function<Resource*(std::string)>> m_FactoryFunctions; // type -> factory function
	static std::unordered_map<std::pair<std::string, std::string>, Resource*> m_ResourceCache; // (type, name) -> resource
	static std::unordered_map<std::string, Resource*> m_ResourceFromName; // name -> resource
	static std::unordered_map<Resource*, Resource*> m_ResourceParents; // resource -> parent resource

	static std::unordered_map<std::pair<std::string, std::string>, boost::thread> m_LoadingThreads; // (type, name) -> loading thread
    static boost::recursive_mutex m_Mutex;

	// TODO: Getters for IDs
	static unsigned int m_CurrentResourceTypeID;
	static std::unordered_map<std::string, unsigned int> m_ResourceTypeIDs;
	// Number of resources of a type. Doubles as local ID.
	static std::unordered_map<unsigned int, unsigned int> m_ResourceCount;

	static FileWatcher m_FileWatcher;
	static void fileWatcherCallback(std::string path, FileWatcher::FileEventFlags flags);

	static unsigned int GetTypeID(std::string resourceType);
	static unsigned int GetNewResourceID(unsigned int typeID);

	// Internal: Create a resource and cache it
	static Resource* createResource(std::string resourceType, std::string resourceName, Resource* parent);

    static bool IsMainThread();
};

template <typename T>
void ResourceManager::RegisterType(std::string typeName)
{
	m_CompilerTypenameToResourceType[typeid(T).name()] = typeName;
	m_FactoryFunctions[typeName] = [](std::string resourceName) { return new T(resourceName); };
}

template <typename T, bool async>
static T* ResourceManager::Load(std::string resourceName, Resource* parent /* = nullptr */)
{
    auto resourceTypename = typeid(T).name();
    auto iter = m_CompilerTypenameToResourceType.find(resourceTypename);
    if (iter == m_CompilerTypenameToResourceType.end()) {
        LOG_ERROR("Failed to load resource \"%s\" of type \"%s\": type not registered", resourceName.c_str(), resourceTypename);
        return nullptr;
    }

    std::string resourceType = iter->second;
    constexpr bool mustNotLoadInThread = std::is_base_of<ThreadUnsafeResource, T>::value;
    if (mustNotLoadInThread && !IsMainThread()) {
        LOG_ERROR("Failed to Load \"%s\": ThreadUnsafeResource type \"%s\" load in the constructor of another resource that is loaded asyncronously.", resourceName.c_str(), iter->second.c_str());
        return nullptr;
    }

    auto cacheKey = std::make_pair(resourceType, resourceName);
    decltype(m_ResourceCache)::iterator it;
    //If a thread has already been launched to load this resource.
    auto tIt = m_LoadingThreads.find(cacheKey);
    if (tIt != m_LoadingThreads.end()) {
        if (async) {
            //Return null if the thread is still working.
            if (!tIt->second.try_join_for(boost::chrono::nanoseconds(1))) {
                return nullptr;
            }
            //Else we know the thread has completed.
        } else {
            //Wait for the thread to finish loading.
            tIt->second.join();
        }
        //When the thread is done, delete the thread.
        m_LoadingThreads.erase(tIt);
        //Find the resource that the thread loaded.
        it = m_ResourceCache.find(cacheKey);
        if (it != m_ResourceCache.end()) {
            //Threads should not be able to throw StillLoadingException, so no check should be needed.
            return static_cast<T*>(it->second);
        } else {
            //If cacheKey does not exist after thread finishes, it failed.
            return nullptr;
        }
    }

    //If resource has already been cached and completely loaded.
    it = m_ResourceCache.find(cacheKey);
    if (it != m_ResourceCache.end() && it->second != m_StillLoading) {
        return static_cast<T*>(it->second);
    }

    Resource* res = nullptr;
    //If resource is not cached..
    if (async) {
        if (mustNotLoadInThread) {
            res = createResource(resourceType, resourceName, parent);
            if (res != m_StillLoading) {
                return static_cast<T*>(res);
            }
        } else {
            //Create a thread that loads the resource into cache.
            m_LoadingThreads[cacheKey] = boost::thread(createResource, resourceType, resourceName, parent);
        }
        return nullptr;
    } else {
        //load and return the resource.
        do {
            res = createResource(resourceType, resourceName, parent);
        } while (res == m_StillLoading);
        return static_cast<T*>(res);
    }
}

#endif
