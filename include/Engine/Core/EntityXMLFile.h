#ifndef EntityXMLFile_h__
#define EntityXMLFile_h__

#include <sstream>

#include "../Common.h"
#include "../GLM.h"
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMLSInput.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLFloat.hpp>
#include <xercesc/framework/XMLGrammarPoolImpl.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/psvi/XSElementDeclaration.hpp>
#include <xercesc/framework/psvi/XSComplexTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSParticle.hpp>
#include <xercesc/framework/psvi/XSModelGroup.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/framework/psvi/XSValue.hpp>
#include <xercesc/framework/XMLValidator.hpp>

#include "ResourceManager.h"
#include "Entity.h"
#include "ComponentInfo.h"
class World;

class EntityPreprocessorXMLErrorHandler : public xercesc::DOMErrorHandler
{
public:
	bool handleError(const xercesc::DOMError &e) override
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		std::cerr << "Preprocessor DOMError: " << message << std::endl;
		xercesc::XMLString::release(&message);
		return false;
	}
};

class EntityParserXMLErrorHandler : public xercesc::ErrorHandler
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

class XSTR
{
public:
	XSTR(const XMLCh* const xmlString) 
	{
		m_AsChar = xercesc::XMLString::transcode(xmlString);
	}

	XSTR(const char* normalString)
	{
		m_AsXMLCh = xercesc::XMLString::transcode(normalString);
	}

	~XSTR()
	{
		if (m_AsChar != nullptr) {
			xercesc::XMLString::release(&m_AsChar);
		}
		if (m_AsXMLCh != nullptr) {
			xercesc::XMLString::release(&m_AsXMLCh);
		}
	}

	operator const char*() const { return m_AsChar; }
	operator const XMLCh*() const { return m_AsXMLCh; }

private:
	char* m_AsChar = nullptr;
	XMLCh* m_AsXMLCh = nullptr;
};

class EntityXMLFile : public Resource
{
	friend class ResourceManager;

private:
	EntityXMLFile(std::string path);

public:
    ~EntityXMLFile();

    void PopulateWorld(World* world);

private:
    static unsigned int InstanceCount;

	std::string m_EntityFile;
	xercesc::XMLGrammarPool* m_GrammarPool = nullptr;
	EntityParserXMLErrorHandler* m_ErrorHandler = nullptr;
	xercesc::XercesDOMParser* m_DOMParser = nullptr;
	xercesc::DOMDocument* m_DOMDocument = nullptr;
	std::map<std::string, ComponentInfo> m_ComponentInfo;

    // Preprocesses the entity file to insert include-by-copy child entities
    // TODO: Make this work in memory instead of saving to file
    void preprocess(std::string inPath, std::string outPath);

    void parseComponentInfo();
    void parseDefaults();
    void predictComponentAllocation();
    void parseEntityGraph();
    std::size_t getTypeStride(std::string typeName);
};

#endif