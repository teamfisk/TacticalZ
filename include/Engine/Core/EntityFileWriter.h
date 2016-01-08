#ifndef EntityFileWriter_h__
#define EntityFileWriter_h__

#include <boost/filesystem.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>

#include "Util/XercesString.h"
#include "EntityFile.h"
#include "World.h"

class EntityFileWriter
{
public:
    EntityFileWriter(boost::filesystem::path file)
        : m_FilePath(file)
    {
        using namespace xercesc;
        m_DOMImplementation = DOMImplementationRegistry::getDOMImplementation(XS::ToXMLCh("LS"));
        m_DOMLSSerializer = static_cast<DOMImplementationLS*>(m_DOMImplementation)->createLSSerializer();
        m_DOMLSSerializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true);
        m_DOMLSSerializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
    }

    void WriteWorld(World* world);
    void WriteEntity(World* world, EntityID entity);

private:
    boost::filesystem::path m_FilePath;
    xercesc::DOMImplementation* m_DOMImplementation;
    xercesc::DOMLSSerializer* m_DOMLSSerializer;

    void appendEntityChildren(xercesc::DOMElement* parentElement, const World* world, EntityID entity);
    void appentEntityComponents(xercesc::DOMElement* parentElemetn, const World* world, EntityID entity);
};

#endif