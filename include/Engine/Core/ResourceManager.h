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
	Implement Create() to return a new object of that type.
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

	/** Preloads a resource and caches it for future use

		@tparam T Resource type.
		@param resourceName Fully qualified name of the resource to preload.
	*/
	template <typename T>
	static void Preload(std::string resourceName);
	static void Preload(std::string resourceType, std::string resourceName);

	/** Checks if a resource is in cache

		@param resourceType Resource type as string.
		@param resourceName Fully qualified name of the resource to preload.
	*/
	// TODO: Templateify
	static bool IsResourceLoaded(std::string resourceType, std::string resourceName);

	/** Hot-loads a resource and caches it for future use

		@tparam T Resource type.
		@param resourceName Fully qualified name of the resource to load.
	*/
	template <typename T>
	static T* Load(std::string resourceName, Resource* parent = nullptr);
	static Resource* Load(std::string resourceType, std::string resourceName, Resource* parent = nullptr);

	/** Reloads an already loaded resource, keeping its resource ID intact.

	 	@tparam T Resource type.
	 	@param resourceName Fully qualified name of the resource to reload.
	*/
    static void Reload(std::string resourceName);

	static void Update();

private:
	static std::unordered_map<std::string, std::string> m_CompilerTypenameToResourceType;
    static std::unordered_map<std::string, std::function<Resource*(std::string)>> m_FactoryFunctions; // type -> factory function
	static std::unordered_map<std::pair<std::string, std::string>, Resource*> m_ResourceCache; // (type, name) -> resource
	static std::unordered_map<std::string, Resource*> m_ResourceFromName; // name -> resource
	static std::unordered_map<Resource*, Resource*> m_ResourceParents; // resource -> parent resource

	// TODO: Getters for IDs
	static unsigned int m_CurrentResourceTypeID;
	static std::unordered_map<std::string, unsigned int> m_ResourceTypeIDs;
	// Number of resources of a type. Doubles as local ID.
	static std::unordered_map<unsigned int, unsigned int> m_ResourceCount;
	// Flag to suppress hot-load warnings when a preloading resource chain loads another resource
	static bool m_Preloading;

	static FileWatcher m_FileWatcher;
	static void fileWatcherCallback(std::string path, FileWatcher::FileEventFlags flags);

	static unsigned int GetTypeID(std::string resourceType);
	static unsigned int GetNewResourceID(unsigned int typeID);

	// Internal: Create a resource and cache it
	static Resource* CreateResource(std::string resourceType, std::string resourceName, Resource* parent);
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

	return static_cast<T*>(Load(it->second, resourceName, parent));
}

template <typename T>
void ResourceManager::RegisterType(std::string typeName)
{
	m_CompilerTypenameToResourceType[typeid(T).name()] = typeName;
	m_FactoryFunctions[typeName] = [](std::string resourceName) { return new T(resourceName); };
}

template <typename T>
void ResourceManager::Preload(std::string resourceName)
{
	auto resourceTypename = typeid(T).name();
	auto it = m_CompilerTypenameToResourceType.find(resourceTypename);
	if (it == m_CompilerTypenameToResourceType.end()) {
		LOG_ERROR("Failed to load resource \"%s\" of type \"%s\": type not registered", resourceName.c_str(), resourceTypename);
		return;
	}

	Preload(it->second, resourceName);
}

#endif
