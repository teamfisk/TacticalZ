#include "Core/EntityFile.h"

EntityFile::EntityFile(std::string path)
{
    EntityXMLFile* xml = ResourceManager::Load<EntityXMLFile>(path);

    EntityXMLFilePreprocessor preprocessor(xml);
    preprocessor.RegisterComponents(this);

    EntityXMLFileParser parser(xml);
    m_RootEntity = parser.MergeEntities(this);

    if (m_RootEntity == EntityID_Invalid) {
        ResourceManager::Release("EntityXMLFile", path);
        throw Resource::FailedLoadingException("Failed to merge entities; root entity is invalid");
    }

    ResourceManager::Release("EntityXMLFile", path);
}

EntityWrapper EntityFile::MergeInto(World* other)
{
    auto mapping = other->Merge(this);
    return EntityWrapper(other, mapping.at(m_RootEntity));
}