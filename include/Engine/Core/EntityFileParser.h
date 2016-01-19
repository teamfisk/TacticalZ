#ifndef EntityFileParser_h__
#define EntityFileParser_h__

#include "EntityFile.h"
#include "World.h"

class EntityFileParser
{
public:
    EntityFileParser(const EntityFile* entityFile);

    EntityID MergeEntities(World* world, EntityID baseParent = EntityID_Invalid);

private:
    const EntityFile* m_EntityFile;
    EntityFileHandler m_Handler;
    World* m_World = nullptr;
    EntityID m_FirstEntity = EntityID_Invalid;
    // Maps EntityIDs local to the file to real IDs in the world after they've been 
    // created in order to resolve parent-child relationships.
    std::map<EntityID, EntityID> m_EntityIDMapper;

    void onStartEntity(EntityID entity, EntityID parent, const std::string& name);
    void onStartComponent(EntityID entity, const std::string& component);
    void onStartComponentField(EntityID entity, const std::string& componentType, const std::string& fieldName, const std::map<std::string, std::string>& attributes);
    void onFieldData(EntityID entity, const std::string& componentType, const std::string& fieldName, const char* fieldData);
};

#endif