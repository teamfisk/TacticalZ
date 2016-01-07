#include "EntityFile.h"
#include "World.h"

class EntityFileParser
{
public:
    EntityFileParser(const EntityFile* entityFile);

    void MergeEntities(World* world);

private:
    const EntityFile* m_EntityFile;
    EntityFileHandler m_Handler;
    World* m_World = nullptr;
    // Maps EntityIDs local to the file to real IDs in the world after they've been 
    // created in order to resolve parent-child relationships.
    std::map<EntityID, EntityID> m_EntityIDMapper;

    void onStartEntity(EntityID entity, EntityID parent);
    void onStartComponent(EntityID entity, std::string component);
    void onStartComponentField(EntityID entity, std::string componentType, std::string fieldName, std::map<std::string, std::string> attributes);
    void onFieldData(EntityID entity, std::string componentType, std::string fieldName, const char* fieldData);
};