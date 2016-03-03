#include "Core/EntityXMLFileParser.h"

EntityXMLFileParser::EntityXMLFileParser(const EntityXMLFile* entityFile)
    : m_EntityFile(entityFile)
{
    m_Handler.SetStartEntityCallback(std::bind(&EntityXMLFileParser::onStartEntity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_Handler.SetStartComponentCallback(std::bind(&EntityXMLFileParser::onStartComponent, this, std::placeholders::_1, std::placeholders::_2));
    m_Handler.SetStartFieldCallback(std::bind(&EntityXMLFileParser::onStartComponentField, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_Handler.SetStartFieldDataCallback(std::bind(&EntityXMLFileParser::onFieldData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

EntityID EntityXMLFileParser::MergeEntities(World* world, EntityID baseParent /*= EntityID_Invalid */)
{
    m_World = world;
    m_EntityIDMapper[0] = baseParent;
    m_EntityFile->Parse(&m_Handler);
    return m_FirstEntity;
}

void EntityXMLFileParser::onStartEntity(EntityID entity, EntityID parent, const std::string& name)
{
    EntityID realParent = m_EntityIDMapper.at(parent);
    EntityID realEntity = m_World->CreateEntity(realParent);
    if (m_FirstEntity == EntityID_Invalid) {
        m_FirstEntity = realEntity;
    }
    if (!name.empty()) {
        m_World->SetName(realEntity, name);
    }
    m_EntityIDMapper[entity] = realEntity;
    //LOG_DEBUG("Created entity #%i (%i) with parent %i (%i)", entity, realEntity, parent, realParent);
}

void EntityXMLFileParser::onStartComponent(EntityID entity, const std::string& component)
{
    if (m_World->GetComponentPools().count(component) != 0) {
        EntityID realEntity = m_EntityIDMapper.at(entity);
        m_World->AttachComponent(realEntity, component);
    } else {
        LOG_ERROR("Tried to attach unregistered component \"%s\"! to entity #%i. Ignoring.", component.c_str(), entity);
    }
    //LOG_DEBUG("Attached component of type \"%s\" to entity #%i (%i)", component.c_str(), entity, realEntity);
}

void EntityXMLFileParser::onStartComponentField(EntityID entity, const std::string& componentType, const std::string& fieldName, const std::map<std::string, std::string>& attributes)
{
    if (m_World->GetComponentPools().count(componentType) == 0) {
        return;
    }
    EntityID realEntity = m_EntityIDMapper.at(entity);
    ComponentWrapper component = m_World->GetComponent(realEntity, componentType);
    auto fieldIt = component.Info.Fields.find(fieldName);
    if (fieldIt == component.Info.Fields.end()) {
        LOG_ERROR("Tried to set unknown field \"%s\" of component type \"%s\"! Ignoring.", fieldName.c_str(), componentType.c_str());
        return;
    }
    auto& field = fieldIt->second;

    //LOG_DEBUG("Field \"%s\" type \"%s\"", fieldName.c_str(), field.Type.c_str());
    //LOG_DEBUG("Attributes:");
    //for (auto& kv : attributes) {
    //    LOG_DEBUG("\t%s = %s", kv.first.c_str(), kv.second.c_str());
    //}

    char* data = component.Data + field.Offset;
    EntityXMLFile::WriteAttributeData(data, field, attributes);
}

void EntityXMLFileParser::onFieldData(EntityID entity, const std::string& componentType, const std::string& fieldName, const char* fieldData)
{
    if (m_World->GetComponentPools().count(componentType) == 0) {
        return;
    }
    EntityID realEntity = m_EntityIDMapper.at(entity);
    ComponentWrapper component = m_World->GetComponent(realEntity, componentType);
    auto fieldIt = component.Info.Fields.find(fieldName);
    if (fieldIt == component.Info.Fields.end()) {
        return;
    }
    auto& field = fieldIt->second;

    char* data = component.Data + field.Offset;
    EntityXMLFile::WriteValueData(data, field, fieldData);
}
