#ifndef EntityFile_h__
#define EntityFile_h__

#include <stack>
#include <boost/lexical_cast.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/psvi/XSElementDeclaration.hpp>
#include <xercesc/framework/psvi/XSComplexTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSParticle.hpp>
#include <xercesc/framework/psvi/XSModelGroup.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/framework/psvi/XSValue.hpp>
#include <xercesc/framework/XMLGrammarPoolImpl.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMLSInput.hpp>
#include <xercesc/dom/DOMElement.hpp>

#include "../GLM.h"
#include "Util/XercesString.h"
#include "ResourceManager.h"
#include "Entity.h"
#include "ComponentInfo.h"

class EntityFileHandler
{
    friend class EntityFileSAXHandler;
public:
    // @param EntityID The entity found
    // @param EntityID The parent of the entity
    typedef std::function<void(EntityID, EntityID, const std::string&)> OnStartEntityCallback;
    void SetStartEntityCallback(OnStartEntityCallback c) { m_OnStartEntityCallback = c; }
    // @param EntityID The entity the component corresponds to
    // @param std::string Type name of the component
    typedef std::function<void(EntityID, const std::string&)> OnStartComponentCallback;
    void SetStartComponentCallback(OnStartComponentCallback c) { m_OnStartComponentCallback = c; }
    // @param EntityID Entity 
    // @param std::string Component name 
    // @param std::string Field name
    // @param std::map<std::string, std::string> Field attribute names and values
    typedef std::function<void(EntityID, const std::string&, const std::string&, const std::map<std::string, std::string>&)> OnStartFieldCallback;
    void SetStartFieldCallback(OnStartFieldCallback c) { m_OnStartFieldCallback = c; }
    // @param EntityID Entity 
    // @param std::string Component name 
    // @param std::string Field name
    // @param char* Field data
    typedef std::function<void(EntityID, const std::string&, const std::string&, const char*)> OnStartFieldDataCallback;
    void SetStartFieldDataCallback(OnStartFieldDataCallback c) { m_OnStartFieldDataCallback = c; }

private:
    OnStartEntityCallback m_OnStartEntityCallback = nullptr;
    OnStartComponentCallback m_OnStartComponentCallback = nullptr;
    OnStartFieldCallback m_OnStartFieldCallback = nullptr;
    OnStartFieldDataCallback m_OnStartFieldDataCallback = nullptr;
};

class EntityFileSAXHandler : public xercesc::DefaultHandler
{
public:
    enum class State
    {
        Unknown,
        Entity,
        Component,
        ComponentField
    };

    EntityFileSAXHandler(const EntityFileHandler* handler, xercesc::SAX2XMLReader* reader);

    void startElement(const XMLCh* const _uri, const XMLCh* const _localName, const XMLCh* const _qname, const xercesc::Attributes& attrs) override;
    void characters(const XMLCh* const chars, const XMLSize_t length) override;
    void endElement(const XMLCh* const _uri, const XMLCh* const _localName, const XMLCh* const _qname) override;

    void warning(const xercesc::SAXParseException& e);
    void error(const xercesc::SAXParseException& e);
    void fatalError(const xercesc::SAXParseException& e);

private:
    const EntityFileHandler* m_Handler;
    xercesc::XMLGrammarPool* m_GrammarPool;
    xercesc::SAX2XMLReader* m_Reader;
    //State m_CurrentScope = State::Unknown;
    std::stack<State> m_StateStack;
    unsigned int m_NextEntityID = 0;
    std::stack<EntityID> m_EntityStack;
    std::string m_CurrentComponent;
    std::string m_CurrentField;
    std::map<std::string, std::string> m_CurrentAttributes;

    void onStartEntity(const xercesc::Attributes& attrs); 
    void onEndEntity();
    void onStartEntityRef(const xercesc::Attributes& attrs);
    void onStartComponent(const std::string& name);
    void onEndComponent(const std::string& name);
    void onStartComponentField(const std::string& field, const xercesc::Attributes& attrs);
    void onEndComponentField(const std::string& field);
    void onFieldData(char* data);
};

class EntityFileXMLErrorHandler : public xercesc::ErrorHandler
{
public:
	void warning(const xercesc::SAXParseException& e) override
	{
		reportParseException("Warning", e);
	}
	void error(const xercesc::SAXParseException& e) override
	{
		reportParseException("Error", e);
	}
	void fatalError(const xercesc::SAXParseException& e) override
	{
		reportParseException("FATAL ERROR", e);
	}
	void resetErrors() override { }

private:
	void reportParseException(std::string type, const xercesc::SAXParseException& e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		char* systemID = xercesc::XMLString::transcode(e.getSystemId());
		std::cerr << systemID << ":" << e.getLineNumber() << ":" << e.getColumnNumber() << std::endl;
		std::cerr << type << ": " << message << std::endl;
		xercesc::XMLString::release(&systemID);
		xercesc::XMLString::release(&message);
	}
};

class EntityXMLFile : public Resource
{
    friend class ResourceManager;
    friend class EntityFileSAXHandler;
private:
    EntityXMLFile(boost::filesystem::path path);
    ~EntityXMLFile();

public:
    static unsigned int GetTypeStride(std::string typeName);
    static void WriteAttributeData(char* outData, const ComponentInfo::Field_t& field, const std::map<std::string, std::string>& attributes);
    static void WriteValueData(char* outData, const ComponentInfo::Field_t& field, const char* valueData);

    xercesc::XMLGrammarPool* GrammarPool() const { return m_GrammarPool; }

    void Parse(const EntityFileHandler* handler) const;

private:
    boost::filesystem::path m_FilePath;
    xercesc::XMLGrammarPool* m_GrammarPool;
    xercesc::SAX2XMLReader* m_SAX2XMLReader;
	//std::map<std::string, ::ComponentInfo> m_ComponentInfo;
    //std::vector<std::string> m_EntityReferences;

    static void setReaderFeatures(xercesc::SAX2XMLReader* reader);
};

#endif