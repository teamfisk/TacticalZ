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
	
    If it should be possible to load the resource asyncronously (on a separate thread in the background), 
    using LoadAsync() then any necessary OpenGL calls (Eg. glGenBuffers(), glBindBuffer(), etc.) must be 
    made in GlCommands() method, and not inside the constructor (see Texture.cpp for an example).
*/
class Resource
{
	friend class ResourceManager;

private:
    bool m_FullyConstructed;

protected:
	Resource() : m_FullyConstructed(false) { }
    //This method only needs to be overridden if: 
    //      1:  a) It should be possible to load the resource with LoadAsync(), or
    //          b) This resource will be loaded inside the constructor of another resource that can be loaded with LoadAsync().
    //          c) Same as above but recursively.
    //      2:  a) The resource needs to make OpenGL calls, or
    //          b) The resource contains a resource that makes OGL calls, or
    //          c) The resource contains a resource that contains ... ... a resource that makes OGL calls, or
    //
    //If 2.a: any calls should be made in the GlCommands.
    //If 2.b or 2.c: PostCtorGLCommands() should be called on the contained resource(s).
    //If GlCommands needs to be overridden and GlCommands is also overridden by a baseclass then the baseclass 
    //implementation should be called in from the derived class implementation (see Model.cpp for an example).
    virtual void GlCommands() { }

public:
	// Pretend that this is a pure virtual function that you have to implement
	// FIXME: Why did we do this again instead of just using the constructor?
	// static Resource* Create(std::string resourceName);

	virtual void Reload() {	}
	virtual void OnChildReloaded(Resource* child) { }
    //If this resource implements GlCommands and it is loaded in another resource, 
    //then the containing resource must also implement GlCommands and 
    //call this method in it (see RawModel.cpp for an example).
    void PostCtorGLCommands() 
    {
        if (!m_FullyConstructed) {
            GlCommands();
        }
        m_FullyConstructed = true;
    }

	unsigned int TypeID;
	unsigned int ResourceID;
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

    /** If the resource is not in cache, starts loading the resource and returns nullptr immediately.
        If the resource has been loaded already, return a pointer to it.

        @tparam T Resource type.
        @param resourceName Fully qualified name of the resource to load.
    */
    template <typename T>
    static T* LoadAsync(std::string resourceName, Resource* parent = nullptr);

	/** Hot-loads a resource and caches it for future use. 
        Fairly safe to assume that return value is a valid pointer, will only return nullptr on error.

		@tparam T Resource type.
		@param resourceName Fully qualified name of the resource to load.
	*/
	template <typename T>
	static T* Load(std::string resourceName, Resource* parent = nullptr);

	/** Reloads an already loaded resource, keeping its resource ID intact.

	 	@tparam T Resource type.
	 	@param resourceName Fully qualified name of the resource to reload.
	*/
    static void Reload(std::string resourceName);
    
    static void Release(std::string resourceType, std::string resourceName);

	static void Update();

private:
    //This is a haxy way to make sure that IsMainThread is run when the program starts, so the master thread id is set.
    struct MasterThreadChecker
    {
        MasterThreadChecker()
        {
            ResourceManager::IsMainThread();
        }
    };
    static MasterThreadChecker m_Checker;

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
    template <bool Async>
    static Resource* Load(std::string resourceType, std::string resourceName, Resource* parent = nullptr);
};

template <typename T>
T* ResourceManager::Load(std::string resourceName, Resource* parent /* = nullptr */)
{
    auto resourceTypename = typeid(T).name();
    auto it = m_CompilerTypenameToResourceType.find(resourceTypename);
    if (it == m_CompilerTypenameToResourceType.end()) {
        LOG_ERROR("Failed to load resource \"%s\" of type \"%s\": type not registered", resourceName.c_str(), resourceTypename);
        return nullptr;
    }

    return static_cast<T*>(Load<false>(it->second, resourceName, parent));
}

template <typename T>
void ResourceManager::RegisterType(std::string typeName)
{
	m_CompilerTypenameToResourceType[typeid(T).name()] = typeName;
	m_FactoryFunctions[typeName] = [](std::string resourceName) { return new T(resourceName); };
}

template <typename T>
T* ResourceManager::LoadAsync(std::string resourceName, Resource* parent /* = nullptr */)
{
	auto resourceTypename = typeid(T).name();
	auto it = m_CompilerTypenameToResourceType.find(resourceTypename);
	if (it == m_CompilerTypenameToResourceType.end()) {
		LOG_ERROR("Failed to load resource \"%s\" of type \"%s\": type not registered", resourceName.c_str(), resourceTypename);
		return nullptr;
	}

	return static_cast<T*>(Load<true>(it->second, resourceName, parent));
}

template <bool Async>
static Resource* ResourceManager::Load(std::string resourceType, std::string resourceName, Resource* parent /* = nullptr */)
{
    auto cacheKey = std::make_pair(resourceType, resourceName);
    decltype(m_ResourceCache)::iterator it;
    //If a thread has already been launched to load this resource.
    auto tIt = m_LoadingThreads.find(cacheKey);
    if (tIt != m_LoadingThreads.end()) {
        if (Async) {
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
            //At this point the resource may not be completely 
            //done since the worker threads cannot do opengl commands.
            if (IsMainThread()) {
                //Do the gl commands to complete the resource if we are not a worker thread.
                it->second->PostCtorGLCommands();
            }
            return it->second;
        } else {
            //If resource is still null at cacheKey after thread finishes, it failed.
            return nullptr;
        }
    }

    //If resource has already been loaded and cached.
    it = m_ResourceCache.find(cacheKey);
    if (it != m_ResourceCache.end()) {
        return it->second;
    }

    //If resource is not cached..
    if (Async) {
        //Create a thread that loads the resource into cache.
        m_LoadingThreads[cacheKey] = boost::thread(createResource, resourceType, resourceName, parent);
        return nullptr;
    } else {
        //load and return the resource.
        Resource* res = createResource(resourceType, resourceName, parent);
        if (IsMainThread() && res != nullptr) {
            //Do the gl commands to complete the resource if we are not a worker thread.
            res->PostCtorGLCommands();
        }
        return res;
    }
}

#endif
