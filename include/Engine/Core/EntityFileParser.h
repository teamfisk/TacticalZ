#include "EntityFile.h"
#include "World.h"

class EntityFileParser
{
public:
    EntityFileParser(EntityFile* entityFile)
        : m_EntityFile(entityFile)
    {
		m_Handler.SetStartEntityCallback(std::bind(&EntityFileParser::onStartEntity, this, std::placeholders::_1, std::placeholders::_2));
		m_Handler.SetStartComponentCallback(std::bind(&EntityFileParser::onStartComponent, this, std::placeholders::_1, std::placeholders::_2));
		m_Handler.SetStartFieldCallback(std::bind(&EntityFileParser::onStartComponentField, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		m_Handler.SetStartFieldDataCallback(std::bind(&EntityFileParser::onFieldData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    }

    void MergeEntities(World* world)
    {
        m_World = world;
        m_EntityIDMapper[0] = 0;
        m_EntityFile->Parse(&m_Handler);
    }

private:
    EntityFile* m_EntityFile;
    EntityFileHandler m_Handler;
    World* m_World = nullptr;
    // Maps EntityIDs local to the file to real IDs in the world
    std::map<EntityID, EntityID> m_EntityIDMapper;

    void onStartEntity(EntityID entity, EntityID parent)
    {
        EntityID realParent = m_EntityIDMapper.at(parent);
        EntityID realEntity = m_World->CreateEntity(realParent);
        m_EntityIDMapper[entity] = realEntity;
        LOG_DEBUG("Created entity #%i (%i) with parent %i (%i)", entity, realEntity, parent, realParent);
    }

    void onStartComponent(EntityID entity, std::string component)
    {
        EntityID realEntity = m_EntityIDMapper.at(entity);
        m_World->AttachComponent(entity, component);
        LOG_DEBUG("Attached component of type \"%s\" to entity #%i (%i)", component.c_str(), entity, realEntity);
    }
    
    void onStartComponentField(EntityID entity, std::string componentType, std::string fieldName, std::map<std::string, std::string> attributes)
    {
        EntityID realEntity = m_EntityIDMapper.at(entity);
        ComponentWrapper component = m_World->GetComponent(realEntity, componentType);
        std::string fieldType = component.Info.FieldTypes.at(fieldName);

        LOG_DEBUG("Field \"%s\" type \"%s\"", fieldName.c_str(), fieldType.c_str());
        LOG_DEBUG("Attributes:");
        for (auto& kv : attributes) {
            LOG_DEBUG("\t%s = %s", kv.first.c_str(), kv.second.c_str());
        }

        char* data = component.Data + component.Info.FieldOffsets.at(fieldName);
        if (fieldType == "Vector") {
            glm::vec3 vec;
            vec.x = boost::lexical_cast<float>(attributes.at("X"));
            vec.y = boost::lexical_cast<float>(attributes.at("Y"));
            vec.z = boost::lexical_cast<float>(attributes.at("Z"));
            memcpy(data, reinterpret_cast<char*>(&vec), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "Color") {
            glm::vec4 vec;
            vec.r = boost::lexical_cast<float>(attributes.at("R"));
            vec.g = boost::lexical_cast<float>(attributes.at("G"));
            vec.b = boost::lexical_cast<float>(attributes.at("B"));
            vec.a = boost::lexical_cast<float>(attributes.at("A"));
            memcpy(data, reinterpret_cast<char*>(&vec), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "Quaternion") {
            glm::quat q;
            q.x = boost::lexical_cast<float>(attributes.at("X"));
            q.y = boost::lexical_cast<float>(attributes.at("Y"));
            q.z = boost::lexical_cast<float>(attributes.at("Z"));
            q.w = boost::lexical_cast<float>(attributes.at("W"));
            memcpy(data, reinterpret_cast<char*>(&q), EntityFile::GetTypeStride(fieldType));
        } else if (!attributes.empty()) {
            LOG_WARNING("%i attributes not handled by any type conversion!", attributes.size());
        }
    }

    void onFieldData(EntityID entity, std::string componentType, std::string fieldName, const char* fieldData)
    {
        EntityID realEntity = m_EntityIDMapper.at(entity);
        ComponentWrapper component = m_World->GetComponent(realEntity, componentType);
        std::string fieldType = component.Info.FieldTypes.at(fieldName);

        char* data = component.Data + component.Info.FieldOffsets.at(fieldName);
        if (fieldType == "int") {
            int value = boost::lexical_cast<int>(fieldData);
            memcpy(data, reinterpret_cast<char*>(&value), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "float") {
            float value = boost::lexical_cast<float>(fieldData);
            memcpy(data, reinterpret_cast<char*>(&value), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "double") {
            double value = boost::lexical_cast<double>(fieldData);
            memcpy(data, reinterpret_cast<char*>(&value), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "bool") {
            bool value = (fieldData[0] == 't'); // Lazy bool evaluation
            memcpy(data, reinterpret_cast<char*>(&value), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "string") {
            new (data) std::string(fieldData);
        } else {
            LOG_WARNING("Unknown data type: %s", fieldType.c_str());
        }
    }
};