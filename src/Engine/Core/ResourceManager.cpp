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
bool ResourceManager::UseThreading = false;
std::unordered_map<std::string, unsigned int> ResourceManager::m_ResourceTypeIDs;
std::unordered_map<unsigned int, unsigned int> ResourceManager::m_ResourceCount;
FileWatcher ResourceManager::m_FileWatcher;
std::unordered_map<std::pair<std::string, std::string>, boost::thread> ResourceManager::m_LoadingThreads;
std::unordered_map<std::pair<std::string, std::string>, std::exception_ptr> ResourceManager::m_LoadingThreadExceptions;
boost::recursive_mutex ResourceManager::m_Mutex;

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

Resource* ResourceManager::createResource(const std::string& resourceType, const std::string& resourceName, Resource* parent, std::exception_ptr& exception)
{
	auto facIt = m_FactoryFunctions.find(resourceType);
	if (facIt == m_FactoryFunctions.end()) {
		LOG_ERROR("Failed to load resource \"%s\" of type \"%s\": type not registered", resourceName.c_str(), resourceType.c_str());
        cacheResource(nullptr, resourceType, resourceName, parent);
		//This basically throws an exception.
        exception = std::make_exception_ptr(Resource::FailedLoadingException()); return nullptr;
	}

	// Call the factory function
    try {
        return cacheResource(facIt->second(resourceName), resourceType, resourceName, parent);
    } catch (const Resource::StillLoadingException&) {
        exception = std::current_exception(); return nullptr;
    } catch (const std::exception& e) {
		LOG_ERROR("Failed to load resource \"%s\" of type \"%s\": %s", resourceName.c_str(), resourceType.c_str(), e.what());
        cacheResource(nullptr, resourceType, resourceName, parent);
        exception = std::current_exception(); return nullptr;
    }
}


Resource* ResourceManager::createResourceThrowing(const std::string& resourceType, const std::string& resourceName, Resource* parent)
{
    std::exception_ptr exception;
    Resource* res = createResource(resourceType, resourceName, parent, exception);
    if (exception) {
        std::rethrow_exception(exception);
    }
    return res;
}

Resource* ResourceManager::cacheResource(Resource* resource, const std::string& resourceType, const std::string& resourceName, Resource* parent)
{
    //Lock the mutex immediately, and unlock it when leaving the code block.
    boost::lock_guard<decltype(m_Mutex)> guard(m_Mutex);
    if (resource != nullptr) {
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

    //if (!boost::filesystem::is_directory(resourceName)) {
    //    LOG_DEBUG("Adding watch for %s", resourceName.c_str());
    //    m_FileWatcher.AddWatch(resourceName, fileWatcherCallback);
    //}
    return resource;
}

bool ResourceManager::IsMainThread()
{
    static boost::thread::id MainThreadId = boost::this_thread::get_id();
    return boost::this_thread::get_id() == MainThreadId;
}
