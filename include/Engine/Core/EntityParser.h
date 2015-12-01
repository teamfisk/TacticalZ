#include "../Common.h"
#include <sstream>
#include "../GLM.h"

#include <boost/filesystem.hpp>
#include <boost/utility/string_ref.hpp>

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

#include "Entity.h"
#include "Component.h"

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

struct ComponentPool
{
	std::string ComponentName;
	unsigned int Size = 0;
	unsigned int Stride = 0;
	ComponentInfo Info;
	char* Data = nullptr;

	// TODO: Iterators
	Component at(unsigned int index)
	{
		// TODO: EntityID
		return Component(0, &Info, Data + (index*Stride));
	}
};

class EntityParser
{
public:
	EntityParser(std::string entityFile)
		: m_EntityFile(entityFile)
	{
		using namespace xercesc;

		if (InstanceCount == 0) {
			XMLPlatformUtils::Initialize();
		}
		InstanceCount++;

		m_GrammarPool = new XMLGrammarPoolImpl();
		m_ErrorHandler = new EntityParserXMLErrorHandler();
		m_DOMParser = new XercesDOMParser(nullptr, XMLPlatformUtils::fgMemoryManager, m_GrammarPool);
		m_DOMParser->setErrorHandler(m_ErrorHandler);
		m_DOMParser->setDoNamespaces(true);
		m_DOMParser->setDoXInclude(true);
		m_DOMParser->setDoSchema(true);
		m_DOMParser->setValidationSchemaFullChecking(true);
		m_DOMParser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
		m_DOMParser->setValidationSchemaFullChecking(true);
		m_DOMParser->setValidationConstraintFatal(false);
		m_DOMParser->setIncludeIgnorableWhitespace(false);
		// Make sure schema grammar is kept after validation
		m_DOMParser->cacheGrammarFromParse(true);
	}

	~EntityParser()
	{
		using namespace xercesc;

		if (m_DOMParser != nullptr) {
			delete m_DOMParser;
		}
		if (m_ErrorHandler != nullptr) {
			delete m_ErrorHandler;
		}
		if (m_GrammarPool != nullptr) {
			delete m_GrammarPool;
		}

		InstanceCount++;
		if (InstanceCount == 0) {
			XMLPlatformUtils::Terminate();
		}
	}


	void Preprocess(boost::filesystem::path inPath, boost::filesystem::path outPath)
	{
		using namespace xercesc;

		static const XMLCh gLS[] = { 'L', 'S', '\0' };
		DOMImplementationLS* di = static_cast<DOMImplementationLS*>(DOMImplementationRegistry::getDOMImplementation(gLS));

		// Parse the file
		DOMLSParser* parser = di->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, nullptr);
		DOMConfiguration* config = parser->getDomConfig();
		config->setParameter(XMLUni::fgDOMNamespaces, true);
		config->setParameter(XMLUni::fgXercesSchema, true);
		config->setParameter(XMLUni::fgXercesHandleMultipleImports, true);
		config->setParameter(XMLUni::fgXercesSchemaFullChecking, true);
		config->setParameter(XMLUni::fgXercesDoXInclude, true);
		auto errHandler = new EntityPreprocessorXMLErrorHandler();
		config->setParameter(XMLUni::fgDOMErrorHandler, errHandler);

		auto source = new LocalFileInputSource(XSTR(inPath.string().c_str()));
		Wrapper4InputSource* domSourceWrapper = new Wrapper4InputSource(source);
		DOMDocument* doc = parser->parse(dynamic_cast<DOMLSInput*>(domSourceWrapper));

		// Serialize and output the new XML
		DOMLSSerializer* writer = di->createLSSerializer();
		DOMLSOutput* output = di->createLSOutput();
		XMLFormatTarget* formatTarget = new LocalFileFormatTarget(outPath.string().c_str());
		// TODO: MemBufFormatTarget* formatTarget = new MemBufFormatTarget()
		output->setByteStream(formatTarget);
		writer->write(doc, output);

		delete formatTarget;
		output->release();
		writer->release();
		parser->release();
	}
	void Parse()
	{
		// HACK: Use Sax2 parser instead so the whole DOM doesn't have to reside in memory
		m_DOMParser->parse(m_EntityFile.c_str());
		m_DOMDocument = m_DOMParser->getDocument();

		// 1. Fill in ComponentInfo name, fields, default values and metadata from PSVI
		ParseComponentInfo();
		// 2. Parse default value files for those components
		ParseDefaults();
		// 3. Allocate component structures
		AllocateComponentStore();
		// 4. Parse entity hierarchy
		ParseEntityGraph();
	}

private:
	static unsigned int InstanceCount;

	std::string m_EntityFile;
	xercesc::XMLGrammarPool* m_GrammarPool = nullptr;
	EntityParserXMLErrorHandler* m_ErrorHandler = nullptr;
	xercesc::XercesDOMParser* m_DOMParser = nullptr;
	xercesc::DOMDocument* m_DOMDocument = nullptr;

	std::map<std::string, ComponentInfo> m_ComponentInfo;
public:
	std::map<std::string, ComponentPool> m_ComponentStore;
	std::vector<Entity*> m_Entities;
private:

	/*
		Preprocess an XML file and output a new one
		where xsi:includes are processed, since apparently
		Xerces can't handle processing includes before validating schema.
	*/

	void ParseComponentInfo()
	{
		using namespace xercesc;
		bool wasChanged;
		XSModel* xsModel = m_GrammarPool->getXSModel(wasChanged);

		// Find component xsd element declarations
		std::cout << "Enumerating components..." << std::endl;
		// <xs:element name="ComponentName">
		auto topLevelElements = xsModel->getComponents(XSConstants::ELEMENT_DECLARATION);
		for (unsigned int i = 0; i < topLevelElements->getLength(); ++i) {
			auto element = static_cast<XSElementDeclaration*>(topLevelElements->item(i));

			std::string nameSpace(XSTR(element->getNamespace()));
			if (nameSpace != "components") {
				continue;
			}

			ComponentInfo compInfo;

			// Name
			compInfo.Name = XSTR(element->getName());
			// Annotation
			auto componentAnnotation = element->getAnnotation();
			if (componentAnnotation != nullptr) {
				// Parse annotation XML
				char* annotationString = XMLString::transcode(componentAnnotation->getAnnotationString());
				MemBufInputSource annotationInput(reinterpret_cast<const XMLByte*>(annotationString), strlen(annotationString), "MemBuf: Annotation String");
				XercesDOMParser parser(nullptr, XMLPlatformUtils::fgMemoryManager, m_GrammarPool);
				parser.setErrorHandler(m_ErrorHandler);
				parser.parse(annotationInput);
				XMLString::release(&annotationString);
				auto doc = parser.getDocument();

				// Add allocation estimation(s)
				auto allocationTags = doc->getElementsByTagName(XSTR("meta:allocation"));
				for (int i = 0; i < allocationTags->getLength(); ++i) {
					auto allocation = dynamic_cast<DOMElement*>(allocationTags->item(i));
					auto child = allocation->getFirstChild();
					if (child == nullptr) {
						continue;
					}

					XSValue::Status status;
					XSValue* val = XSValue::getActualValue(child->getNodeValue(), XSValue::dt_integer, status);
					compInfo.Meta.Allocation += val->fData.fValue.f_int;
				}

				// Save documentation string
				auto documentationTags = doc->getElementsByTagName(XSTR("xs:documentation"));
				if (documentationTags->getLength() != 0) {
					auto child = documentationTags->item(0)->getFirstChild();
					if (child != nullptr) {
						compInfo.Meta.Annotation = XSTR(child->getNodeValue());
					}
				}
				// TODO: Parse annotation string XML
				// compInfo.Meta.Allocation = ...
			} else {
				std::cout << "Warning: Component is missing an annotation!" << std::endl;
			}

			// <xs:complexType>
			auto typeDefinition = element->getTypeDefinition();
			if (typeDefinition->getTypeCategory() != XSTypeDefinition::COMPLEX_TYPE) {
				std::cerr << "Error: Type definition wasn't COMPLEX_TYPE! Skipping." << std::endl;
				continue;
			}
			auto complexTypeDefinition = dynamic_cast<XSComplexTypeDefinition*>(typeDefinition);

			// <xs:all>
			auto modelGroupParticle = complexTypeDefinition->getParticle();
			if (modelGroupParticle->getTermType() != XSParticle::TERM_MODELGROUP) {
				std::cerr << "Error: Model group particle wasn't TERM_MODELGROUP! Skipping." << std::endl;
				continue;
			}
			auto modelGroup = modelGroupParticle->getModelGroupTerm();

			// <xs:element... 
			// <xs:attribute...
			unsigned int fieldOffset = 0;
			auto particles = modelGroup->getParticles();
			for (unsigned int i = 0; i < particles->size(); ++i) {
				auto particle = particles->elementAt(i);
				if (particle->getTermType() != XSParticle::TERM_ELEMENT) {
					std::cerr << "Error: Particle wasn't TERM_ELEMENT! Skipping." << std::endl;
					continue;
				}
				auto elementDeclaration = particle->getElementTerm();

				std::string name = XSTR(elementDeclaration->getName());
				std::string type = XSTR(elementDeclaration->getTypeDefinition()->getName());

				size_t stride = getTypeStride(type);
				if (stride == 0) {
					std::cout << "Warning: Field \"" << name << "\" in component \"" << compInfo.Name << "\" uses unexpected field type \"" << type << "\". Skipping." << std::endl;
					continue;
				}

				compInfo.FieldTypes[name] = type;
				compInfo.FieldOffsets[name] = fieldOffset;
				fieldOffset += getTypeStride(type);
			}

			m_ComponentInfo[compInfo.Name] = compInfo;
		}
	}

	void ParseDefaults()
	{

	}

	void AllocateComponentStore()
	{
		using namespace xercesc;

		auto root = m_DOMDocument->getDocumentElement();

		// Count static instances of components present in entity hierarchy
		auto components = m_DOMDocument->getElementsByTagNameNS(XSTR("components"), XSTR("*"));
		for (int i = 0; i < components->getLength(); ++i) {
			auto component = dynamic_cast<DOMElement*>(components->item(i));

			std::string componentName = XSTR(component->getLocalName());
			auto& compInfo = m_ComponentInfo.at(componentName);
			compInfo.Meta.Allocation += 1;
		}

		std::cout << "COMPONENT INFO" << std::endl;
		for (auto& pair : m_ComponentInfo) {
			ComponentInfo& ci = pair.second;
			std::cout << "Component: " << ci.Name << " (" << ci.Meta.Annotation << ")" << std::endl;
			std::cout << "  Allocation: " << ci.Meta.Allocation << std::endl;
			std::cout << "  Fields:" << std::endl;

			// Calculate component size
			unsigned int stride = 0;
			// Reserve space for Entity pointer
			stride += sizeof(Entity*);
			std::cout << "    Entity " << " (" << sizeof(Entity*) << " byte)" << std::endl;
			// Add size of fields
			for (auto& field : ci.FieldTypes) {
				std::cout << "    " << field.second << " " << field.first << " (" << getTypeStride(field.second) << " byte)" << std::endl;
				stride += getTypeStride(field.second);
			}
			std::cout << "  Stride: " << stride << std::endl;

			ComponentPool cs;
			cs.ComponentName = ci.Name;
			cs.Stride = stride;
			cs.Info = ci;
			cs.Data = new char[stride*ci.Meta.Allocation];
			m_ComponentStore[cs.ComponentName] = cs;
		}
	}

	void ParseEntityGraph()
	{
		using namespace xercesc;

		auto root = m_DOMDocument->getDocumentElement();

		auto components = m_DOMDocument->getElementsByTagNameNS(XSTR("components"), XSTR("*"));
		for (int i = 0; i < components->getLength(); ++i) {
			auto component = dynamic_cast<DOMElement*>(components->item(i));

			std::string componentName = XSTR(component->getLocalName());
			auto& compStore = m_ComponentStore.at(componentName);
			auto& compInfo = compStore.Info;

			char* data = &compStore.Data[compStore.Size*compStore.Stride];
			compStore.Size += 1;

			auto fields = component->getChildNodes();
			for (int j = 0; j < fields->getLength(); ++j) {
				auto field = fields->item(j);
				auto nodeType = field->getNodeType();
				if (nodeType != DOMNode::ELEMENT_NODE) {
					continue;
				}
				//auto field = dynamic_cast<DOMElement*>(fields->item(j));
				//const XMLCh* value = fields->item(j)->getTextContent();
				std::string fieldName = XSTR(field->getLocalName());
				if (compInfo.FieldTypes.find(fieldName) == compInfo.FieldTypes.end()) {
					std::cout << "Warning: Component \"" << componentName << "\" contains invalid field \"" << fieldName << "\". Skipping." << std::endl;
					continue;
				}

				std::string fieldType = compInfo.FieldTypes.at(fieldName);
				unsigned int fieldOffset = compInfo.FieldOffsets.at(fieldName);

				XSValue::DataType dataType = XSValue::getDataType(XSTR(fieldType.c_str()));
				if (dataType == XSValue::DataType::dt_MAXCOUNT) {
					// TODO:
					continue;
				}
				if (dataType == XSValue::DataType::dt_string) {
					char* str = XMLString::transcode(field->getTextContent());
					std::string standardString(str);
					XMLString::release(&str);
					memcpy(&data[fieldOffset], reinterpret_cast<char*>(&standardString), getTypeStride(fieldType));
				} else {
					XSValue::Status status;
					XSValue* val = XSValue::getActualValue(field->getTextContent(), dataType, status);
					memcpy(&data[fieldOffset], reinterpret_cast<char*>(&val->fData.fValue), getTypeStride(fieldType));
				}
			}
		}

		auto entities = m_DOMDocument->getElementsByTagName(XSTR("Entity"));
		for (int i = 0; i < entities->getLength(); ++i) {
			auto entity = dynamic_cast<DOMElement*>(entities->item(i));


			//entity->setIdAttribute()
			
			std::cout << "ENTITY " << i + 1 << std::endl;
		}
	}

	size_t getTypeStride(std::string typeName) 
	{
		std::map<std::string, size_t> typeStrides{
			{ "int", sizeof(int) },
			{ "double", sizeof(double) },
			{ "string", sizeof(std::string) },
			{ "Vector", sizeof(glm::vec3) },
			{ "Quaternion", sizeof(glm::quat) },
		};

		auto it = typeStrides.find(typeName);
		return (it != typeStrides.end()) ? it->second : 0;
	}
};

unsigned int EntityParser::InstanceCount = 0;