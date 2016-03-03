#ifndef EntityXMLFilePreprocessor_h__
#define EntityXMLFilePreprocessor_h__

#include <xercesc/framework/psvi/XSElementDeclaration.hpp>
#include <xercesc/framework/psvi/XSComplexTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSAttributeUse.hpp>
#include <xercesc/framework/psvi/XSAttributeDeclaration.hpp>
#include <xercesc/framework/psvi/XSParticle.hpp>
#include <xercesc/framework/psvi/XSModelGroup.hpp>
#include <xercesc/framework/psvi/XSModelGroupDefinition.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/framework/psvi/XSValue.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include "Util/XercesString.h"
#include "ResourceManager.h"
#include "World.h"
#include "EntityXMLFile.h"

class EntityXMLFilePreprocessor
{
    friend class EntityFile;
public:
    EntityXMLFilePreprocessor(const EntityXMLFile* entityFile);

private:
    void RegisterComponents(World* world);

    const EntityXMLFile* m_EntityFile;
    std::map<std::string, unsigned int> m_ComponentCounts;
	std::map<std::string, ComponentInfo> m_ComponentInfo;

	void onStartComponent(EntityID entity, std::string type);
    void parseComponentInfo();
    void parseDefaults();
    std::string parseAnnotationXML(const XMLCh* xml);
};

#endif