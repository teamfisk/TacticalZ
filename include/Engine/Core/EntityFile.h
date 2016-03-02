#ifndef EntityFile_h__
#define EntityFile_h__

#include "World.h"
#include "EntityXMLFile.h"
#include "EntityXMLFilePreprocessor.h"
#include "EntityXMLFileParser.h"
#include "EntityWrapper.h"

class EntityFile : private World, public Resource
{
public:
    EntityFile(std::string path);

    EntityWrapper MergeInto(World* other);

private:
    EntityID m_RootEntity = EntityID_Invalid;
};

#endif