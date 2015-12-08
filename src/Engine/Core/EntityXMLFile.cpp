#include "Core/EntityXMLFile.h"
#include "Core/World.h"

unsigned int EntityXMLFile::InstanceCount = 0;

EntityXMLFile::EntityXMLFile(std::string path)
    : m_EntityFile(path)
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

    // HACK: Use Sax2 parser instead so the entire DOM doesn't have to reside in memory
    m_DOMParser->parse(m_EntityFile.c_str());
    m_DOMDocument = m_DOMParser->getDocument();

    // 1. Fill in ComponentInfo name, fields, default values and metadata from PSVI
    parseComponentInfo();
    // 2. Parse default value files for those components
    parseDefaults();
    // 3. Allocate component structures
    predictComponentAllocation();
}

EntityXMLFile::~EntityXMLFile()
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

    InstanceCount--;
    if (InstanceCount == 0) {
        XMLPlatformUtils::Terminate();
    }
}

void EntityXMLFile::PopulateWorld(World* world)
{
    for (auto& pair : m_ComponentInfo) {
        world->RegisterComponent(pair.second);
    }

    // 4. Parse entity hierarchy
    //parseEntityGraph();
}


void EntityXMLFile::preprocess(std::string inPath, std::string outPath)
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

    auto source = new LocalFileInputSource(XSTR(inPath.c_str()));
    Wrapper4InputSource* domSourceWrapper = new Wrapper4InputSource(source);
    DOMDocument* doc = parser->parse(dynamic_cast<DOMLSInput*>(domSourceWrapper));

    // Serialize and output the new XML
    DOMLSSerializer* writer = di->createLSSerializer();
    DOMLSOutput* output = di->createLSOutput();
    XMLFormatTarget* formatTarget = new LocalFileFormatTarget(outPath.c_str());
    // TODO: MemBufFormatTarget* formatTarget = new MemBufFormatTarget()
    output->setByteStream(formatTarget);
    writer->write(doc, output);

    delete formatTarget;
    output->release();
    writer->release();
    parser->release();
}

void EntityXMLFile::parseComponentInfo()
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

        compInfo.Meta.Stride = fieldOffset;
        m_ComponentInfo[compInfo.Name] = compInfo;
    }
}

void EntityXMLFile::parseDefaults()
{

}

void EntityXMLFile::predictComponentAllocation()
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
        std::size_t stride = 0;
        // Add size of fields
        for (auto& field : ci.FieldTypes) {
            std::cout << "    " << field.second << " " << field.first << " (" << getTypeStride(field.second) << " byte)" << std::endl;
            stride += getTypeStride(field.second);
        }
        std::cout << "  Stride: " << ci.Meta.Stride << std::endl;

        // TODO: ALLOCATE HERE
        /*ComponentPool cs;
        cs.ComponentName = ci.Name;
        cs.Stride = stride;
        cs.Info = ci;
        cs.Data = new char[stride*ci.Meta.Allocation];
        m_ComponentStore[cs.ComponentName] = cs;*/
    }
}

void EntityXMLFile::parseEntityGraph()
{
    //using namespace xercesc;

    //auto root = m_DOMDocument->getDocumentElement();

    //auto components = m_DOMDocument->getElementsByTagNameNS(XSTR("components"), XSTR("*"));
    //for (int i = 0; i < components->getLength(); ++i) {
    //    auto component = dynamic_cast<DOMElement*>(components->item(i));

    //    std::string componentName = XSTR(component->getLocalName());
    //    auto& compStore = m_ComponentStore.at(componentName);
    //    auto& compInfo = compStore.Info;

    //    char* data = &compStore.Data[compStore.Size*compStore.Stride];
    //    compStore.Size += 1;

    //    auto fields = component->getChildNodes();
    //    for (int j = 0; j < fields->getLength(); ++j) {
    //        auto field = fields->item(j);
    //        auto nodeType = field->getNodeType();
    //        if (nodeType != DOMNode::ELEMENT_NODE) {
    //            continue;
    //        }
    //        //auto field = dynamic_cast<DOMElement*>(fields->item(j));
    //        //const XMLCh* value = fields->item(j)->getTextContent();
    //        std::string fieldName = XSTR(field->getLocalName());
    //        if (compInfo.FieldTypes.find(fieldName) == compInfo.FieldTypes.end()) {
    //            std::cout << "Warning: Component \"" << componentName << "\" contains invalid field \"" << fieldName << "\". Skipping." << std::endl;
    //            continue;
    //        }

    //        std::string fieldType = compInfo.FieldTypes.at(fieldName);
    //        unsigned int fieldOffset = compInfo.FieldOffsets.at(fieldName);

    //        XSValue::DataType dataType = XSValue::getDataType(XSTR(fieldType.c_str()));
    //        if (dataType == XSValue::DataType::dt_MAXCOUNT) {
    //            // TODO:
    //            continue;
    //        }
    //        if (dataType == XSValue::DataType::dt_string) {
    //            char* str = XMLString::transcode(field->getTextContent());
    //            std::string standardString(str);
    //            XMLString::release(&str);
    //            memcpy(&data[fieldOffset], reinterpret_cast<char*>(&standardString), getTypeStride(fieldType));
    //        } else {
    //            XSValue::Status status;
    //            XSValue* val = XSValue::getActualValue(field->getTextContent(), dataType, status);
    //            memcpy(&data[fieldOffset], reinterpret_cast<char*>(&val->fData.fValue), getTypeStride(fieldType));
    //        }
    //    }
    //}

    //auto entities = m_DOMDocument->getElementsByTagName(XSTR("Entity"));
    //for (int i = 0; i < entities->getLength(); ++i) {
    //    auto entity = dynamic_cast<DOMElement*>(entities->item(i));


    //    //entity->setIdAttribute()

    //    std::cout << "ENTITY " << i + 1 << std::endl;
    //}
}

std::size_t EntityXMLFile::getTypeStride(std::string typeName)
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
