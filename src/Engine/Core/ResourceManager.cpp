#include "Core/ResourceManager.h"
#include "boost/thread/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/lock_guard.hpp"

std::unordered_map<std::string, std::string> ResourceManager::m_CompilerTypenameToResourceType;
std::unordered_map<std::string, std::function<Resource*(std::string)>> ResourceManager::m_FactoryFunctions;
std::unordered_map<std::pair<std::string, std::string>, Resource*> ResourceManager::m_ResourceCache;
std::unordered_map<std::string, Resource*> ResourceManager::m_ResourceFromName;
std::unordered_map<Resource*, Resource*> ResourceManager::m_ResourceParents;
unsigned int ResourceManager::m_CurrentResourceTypeID = 0;
std::unordered_map<std::string, unsigned int> ResourceManager::m_ResourceTypeIDs;
std::unordered_map<unsigned int, unsigned int> ResourceManager::m_ResourceCount;
FileWatcher ResourceManager::m_FileWatcher;
std::unordered_map<std::pair<std::string, std::string>, boost::thread> ResourceManager::m_LoadingThreads;
boost::recursive_mutex ResourceManager::m_Mutex;
ResourceManager::SpecialResourcePointer ResourceManager::m_StillLoading;
ResourceManager::SpecialResourcePointer ResourceManager::m_LoadWithMainThread;

unsigned int ResourceManager::GetTypeID(std::string resourceType)
{
	if (m_ResourceTypeIDs.find(resourceType) == m_ResourceTypeIDs.end()) {
		m_ResourceTypeIDs[resourceType] = m_CurrentResourceTypeID++;
	}
	return m_ResourceTypeIDs[resourceType];
}

void ResourceManager::Reload(std::string resourceName)
{
	auto it = m_ResourceFromName.find(resourceName);
	if (it != m_ResourceFromName.end()) {
		LOG_INFO("Reloading resource \"%s\"", resourceName.c_str());
		Resource* resource = it->second;
		resource->Reload();

		// Notify parent
		auto it2 = m_ResourceParents.find(resource);
		if (it2 != m_ResourceParents.end()) {
			it2->second->OnChildReloaded(resource);
		}
	}
}


void ResourceManager::Release(std::string resourceType, std::string resourceName)
{
    auto key = std::make_pair(resourceType, resourceName);
    if (m_ResourceCache.find(key) == m_ResourceCache.end()) {
        return;
    }
    auto resource = m_ResourceCache.at(key);
    m_ResourceCache.erase(key);
    m_ResourceFromName.erase(resourceName);
    m_ResourceParents.erase(resource);
    delete resource;
}

unsigned int ResourceManager::GetNewResourceID(unsigned int typeID)
{
	return m_ResourceCount[typeID]++;
}

bool ResourceManager::IsResourceLoaded(std::string resourceType, std::string resourceName)
{
	return m_ResourceCache.find(std::make_pair(resourceType, resourceName)) != m_ResourceCache.end();
}

void ResourceManager::fileWatcherCallback(std::string path, FileWatcher::FileEventFlags flags)
{
	if (flags & FileWatcher::FileEventFlags::SizeChanged || flags & FileWatcher::FileEventFlags::TimestampChanged) {
		auto it = m_ResourceFromName.find(path);
		if (it != m_ResourceFromName.end()) {
			Reload(path);
		}
	}
}

void ResourceManager::Update()
{
	m_FileWatcher.Check();
}

Resource* ResourceManager::LoadAsync(std::string resourceType, std::string resourceName, Resource* parent /*= nullptr*/)
{
    auto cacheKey = std::make_pair(resourceType, resourceName);
    decltype(m_ResourceCache)::iterator it;
    //If a thread has already been launched to load this resource.
    auto tIt = m_LoadingThreads.find(cacheKey);
    if (tIt != m_LoadingThreads.end()) {
        //If the thread is still working.
        if (tIt->second.joinable()) {
            return nullptr;
        }
        //Else, the thread is done.
        m_LoadingThreads.erase(tIt);
        it = m_ResourceCache.find(cacheKey);
        if (it != m_ResourceCache.end()) {
            //If the thread is done, but it cannot complete the rest, main thread must complete construction.
            if (it->second == m_LoadWithMainThread) {
                AssertIsMainThread();
                return createResource(resourceType, resourceName, parent);
            }
            return it->second;
        } else {
            //If cache is still empty at cacheKey after thread finishes, it failed.
            return nullptr;
        }
    }

    //If resource has already been loaded and cached.
    it = m_ResourceCache.find(cacheKey);
    if (it != m_ResourceCache.end()) {
        //if ConstructByMainThread, createResource from Main.
        return it->second;
    }

    //Create a thread that loads the resource into cache.
    m_LoadingThreads[cacheKey] = boost::thread(createResource, resourceType, resourceName, parent);
    return nullptr;
}

Resource* ResourceManager::Load(std::string resourceType, std::string resourceName, Resource* parent /*= nullptr*/)
{
    Resource* resource;
    auto cacheKey = std::make_pair(resourceType, resourceName);
    decltype(m_ResourceCache)::iterator it;
    //If a thread has already been launched to load this resource.
    auto tIt = m_LoadingThreads.find(cacheKey);
    if (tIt != m_LoadingThreads.end()) {
        //Wait for the thread to finish loading.
        tIt->second.join();
        //Then delete the thread.
        m_LoadingThreads.erase(tIt);
        it = m_ResourceCache.find(cacheKey);
        if (it != m_ResourceCache.end()) {
            //If the thread is done, but it cannot complete the rest, main thread must complete construction.
            if (it->second == m_LoadWithMainThread) {
                AssertIsMainThread();
                return createResource(resourceType, resourceName, parent);
            }
            return it->second;
        } else {
            //If cache is still empty at cacheKey after thread finishes, it failed.
            return nullptr;
        }
    }
    //If resource has already been loaded and cached.
    it = m_ResourceCache.find(cacheKey);
    if (it != m_ResourceCache.end()) {
        return it->second;
    }

    //If resource is not cached, load and return it.
    resource = createResource(resourceType, resourceName, parent);
    if (resource == m_LoadWithMainThread) {
        //If we entered here then we know the caller is a worker thread.
        //And we know the resource must be loaded from master thread.
        throw(WorkerCannotExecute());
    }
    return resource;
}

Resource* ResourceManager::createResource(std::string resourceType, std::string resourceName, Resource* parent)
{
    //Lock the mutex immediately, and unlock it when leaving the function.
    boost::lock_guard<decltype(m_Mutex)> guard(m_Mutex);
	auto facIt = m_FactoryFunctions.find(resourceType);
	if (facIt == m_FactoryFunctions.end()) {
		LOG_ERROR("Failed to load resource \"%s\" of type \"%s\": type not registered", resourceName.c_str(), resourceType.c_str());
		return nullptr;
	}

	// Call the factory function
    Resource* resource = nullptr;
    try {
        resource = facIt->second(resourceName);
    } catch (const WorkerCannotExecute& e) {
        resource = m_LoadWithMainThread;
    } catch (const std::exception& e) {
		LOG_ERROR("Failed to load resource \"%s\" of type \"%s\": %s", resourceName.c_str(), resourceType.c_str(), e.what());
    }
    if (resource != nullptr && resource != m_LoadWithMainThread) {
        // Store IDs
        resource->TypeID = GetTypeID(resourceType);
        resource->ResourceID = GetNewResourceID(resource->TypeID);
    }

	// Cache
	m_ResourceCache[std::make_pair(resourceType, resourceName)] = resource;
	m_ResourceFromName[resourceName] = resource;
	if (parent != nullptr) {
		m_ResourceParents[resource] = parent;
	}

	if (!boost::filesystem::is_directory(resourceName)) {
		LOG_DEBUG("Adding watch for %s", resourceName.c_str());
		m_FileWatcher.AddWatch(resourceName, fileWatcherCallback);
	}
	return resource;
}

bool ResourceManager::IsMainThread()
{
    static boost::thread::id MainThreadId = boost::this_thread::get_id();
    return boost::this_thread::get_id() == MainThreadId;
}

void ResourceManager::AssertIsMainThread()
{
    if (!IsMainThread()) {
        throw WorkerCannotExecute();
    }
}