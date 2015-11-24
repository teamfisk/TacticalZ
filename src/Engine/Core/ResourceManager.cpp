#include "PrecompiledHeader.h"
#include "Core/ResourceManager.h"

std::unordered_map<std::string, std::string> ResourceManager::m_CompilerTypenameToResourceType;
std::unordered_map<std::string, std::function<Resource*(std::string)>> ResourceManager::m_FactoryFunctions;
std::unordered_map<std::pair<std::string, std::string>, Resource*> ResourceManager::m_ResourceCache;
std::unordered_map<std::string, Resource*> ResourceManager::m_ResourceFromName;
std::unordered_map<Resource*, Resource*> ResourceManager::m_ResourceParents;
unsigned int ResourceManager::m_CurrentResourceTypeID = 0;
std::unordered_map<std::string, unsigned int> ResourceManager::m_ResourceTypeIDs;
std::unordered_map<unsigned int, unsigned int> ResourceManager::m_ResourceCount;
bool ResourceManager::m_Preloading = false;
FileWatcher ResourceManager::m_FileWatcher;

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

void ResourceManager::Preload(std::string resourceType, std::string resourceName)
{
	if (IsResourceLoaded(resourceType, resourceName)) {
		//LOG_WARNING("Attempted to preload resource \"%s\" multiple times!", resourceName.c_str());
		return;
	}

	m_Preloading = true;
	LOG_INFO("Preloading resource \"%s\"", resourceName.c_str());
	CreateResource(resourceType, resourceName, nullptr);
	m_Preloading = false;
}

Resource* ResourceManager::Load(std::string resourceType, std::string resourceName, Resource* parent /*= nullptr*/)
{
	auto it = m_ResourceCache.find(std::make_pair(resourceType, resourceName));
	if (it != m_ResourceCache.end()) {
		return it->second;
	}

	if (m_Preloading) {
		LOG_INFO("Preloading resource \"%s\"", resourceName.c_str());
	} else {
		LOG_WARNING("Hot-loading resource \"%s\"", resourceName.c_str());
	}

	return CreateResource(resourceType, resourceName, parent);
}

Resource* ResourceManager::CreateResource(std::string resourceType, std::string resourceName, Resource* parent)
{
	auto facIt = m_FactoryFunctions.find(resourceType);
	if (facIt == m_FactoryFunctions.end()) {
		LOG_ERROR("Failed to load resource \"%s\" of type \"%s\": type not registered", resourceName.c_str(), resourceType.c_str());
		return nullptr;
	}

	// Call the factory function
	Resource* resource = facIt->second(resourceName);
	// Store IDs
	resource->TypeID = GetTypeID(resourceType);
	resource->ResourceID = GetNewResourceID(resource->TypeID);
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
