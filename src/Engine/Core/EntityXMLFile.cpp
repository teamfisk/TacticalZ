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
    auto root = m_DOMDocument->getDocumentElement();
    parseEntityGraph(world, root, 0);
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
    using namespace xercesc;

    for (auto& ci : m_ComponentInfo) {
        // Allocate memory for default values
        ci.second.Defaults = std::shared_ptr<char>(new char[ci.second.Meta.Stride]);
        memset(ci.second.Defaults.get(), 0, ci.second.Meta.Stride);

        XercesDOMParser parser(nullptr, XMLPlatformUtils::fgMemoryManager);
        parser.setErrorHandler(m_ErrorHandler);

        std::string componentName = ci.first;
        LOG_DEBUG("Parsing defaults for component %s", componentName.c_str());
        boost::filesystem::path defaultsFile = "Schema/Components/" + componentName + ".xml";

        parser.parse(defaultsFile.string().c_str());
        auto doc = parser.getDocument();
        if (doc == nullptr) {
            LOG_ERROR("%s not found! Skipping.", defaultsFile.string().c_str());
            continue;
        }

        // Find the node in the components namespace matching the component name
        std::string tagName = "c:" + componentName;
        auto rootNodes = doc->getElementsByTagName(XSTR(tagName.c_str()));
        if (rootNodes->getLength() == 0) {
            LOG_ERROR("Couldn't find defaults for component \"%s\"! Skipping.", componentName.c_str());
            continue;
        }
        auto componentElement = dynamic_cast<DOMElement*>(rootNodes->item(0));
       
        // Fill the default value buffer with values
        for (auto& field : ci.second.FieldOffsets) {
            std::string fieldName = field.first;
            auto fieldNodes = componentElement->getElementsByTagName(XSTR(fieldName.c_str()));
            auto fieldNode = fieldNodes->item(0);
            if (fieldNode == nullptr) {
                LOG_ERROR("Defaults for component \"%s\" is missing field \"%s\"!", componentName.c_str(), fieldName.c_str());
                continue;
            }
            auto fieldElement = dynamic_cast<DOMElement*>(fieldNode);

            std::string fieldType = ci.second.FieldTypes.at(fieldName);
            unsigned int fieldOffset = ci.second.FieldOffsets.at(fieldName);
            writeData(fieldElement, fieldType, ci.second.Defaults.get() + fieldOffset);
        }
    }
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
    }
}

void EntityXMLFile::parseEntityGraph(World* world, xercesc::DOMElement* element, EntityID parentEntity)
{
    using namespace xercesc;

    // Create entity
    EntityID entity = world->CreateEntity(parentEntity);
    LOG_DEBUG("Created entity %i, parent %i", entity, parentEntity);

    // Add components
    auto components = m_DOMDocument->evaluate(XSTR("Components/*"), element, nullptr,  DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE, nullptr);
    for (int i = 0; i < components->getSnapshotLength(); i++) {
        components->snapshotItem(i);
        auto componentElement = dynamic_cast<DOMElement*>(components->getNodeValue());
        std::string componentName = XSTR(componentElement->getLocalName());
        auto& ci = m_ComponentInfo.at(componentName);

        // Attach the component
        auto c = world->AttachComponent(entity, componentName);
        LOG_DEBUG("Attached %s component", componentName.c_str());

        // Write field data
        auto fields = componentElement->getChildNodes();
        for (int j = 0; j < fields->getLength(); ++j) {
            auto fieldNode = fields->item(j);
            auto nodeType = fieldNode->getNodeType();
            if (nodeType != DOMNode::ELEMENT_NODE) {
                continue;
            }
            auto field = dynamic_cast<DOMElement*>(fields->item(j));
            //const XMLCh* value = fields->item(j)->getTextContent();
            std::string fieldName(XSTR(field->getLocalName()));
            if (ci.FieldTypes.find(fieldName) == ci.FieldTypes.end()) {
                std::cout << "Warning: Component \"" << componentName << "\" contains invalid field \"" << fieldName << "\". Skipping." << std::endl;
                continue;
            }

            std::string fieldType = ci.FieldTypes.at(fieldName);
            unsigned int fieldOffset = ci.FieldOffsets.at(fieldName);
            std::string fieldValue(XSTR(field->getTextContent()));
            LOG_DEBUG("    %s %s = %s", fieldType.c_str(), fieldName.c_str(), fieldValue.c_str());
            writeData(field, fieldType, c.Data + fieldOffset);
        }
    }

    // Recurse children
    auto children = m_DOMDocument->evaluate(XSTR("Children/Entity"), element, nullptr, DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE, nullptr);
    for (int i = 0; i < children->getSnapshotLength(); i++) {
        children->snapshotItem(i);
        parseEntityGraph(world, dynamic_cast<DOMElement*>(children->getNodeValue()), entity);
    }

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
        { "bool", sizeof(bool) },
        { "int", sizeof(int) },
        { "float", sizeof(float) },
        { "double", sizeof(double) },
        { "string", sizeof(std::string) },
        { "Vector", sizeof(glm::vec3) },
        { "Quaternion", sizeof(glm::quat) },
        { "Color", sizeof(glm::vec4) }
    };

    auto it = typeStrides.find(typeName);
    return (it != typeStrides.end()) ? it->second : 0;
}

float EntityXMLFile::getFloatAttribute(const xercesc::DOMElement* element, const char* attribute) const
{
    using namespace xercesc;
    XSValue::Status status;
    XSValue* val = XSValue::getActualValue(element->getAttribute(XSTR(attribute)), xercesc::XSValue::DataType::dt_double, status);
    if (val == nullptr) {
        LOG_ERROR("Element \"%s\" doesn't have an \"%s\" attribute!", XSTR(element->getTagName()), attribute);
        return 0.f;
    } else {
        return static_cast<float>(val->fData.fValue.f_double);
    }
}

void EntityXMLFile::writeData(const xercesc::DOMElement* element, std::string typeName, char* outData)
{
    using namespace xercesc;

    if (typeName == "Vector") {
        glm::vec3 vec;
        vec.x = getFloatAttribute(element, "X");
        vec.y = getFloatAttribute(element, "Y");
        vec.z = getFloatAttribute(element, "Z");
        memcpy(outData, reinterpret_cast<char*>(&vec), getTypeStride(typeName));
    } else if (typeName == "Color") {
        glm::vec4 vec;
        vec.r = getFloatAttribute(element, "R");
        vec.g = getFloatAttribute(element, "G");
        vec.b = getFloatAttribute(element, "B");
        vec.a = getFloatAttribute(element, "A");
        memcpy(outData, reinterpret_cast<char*>(&vec), getTypeStride(typeName));
    } else if (typeName == "Quaternion") {
        glm::quat q;
        q.x = getFloatAttribute(element, "X");
        q.y = getFloatAttribute(element, "Y");
        q.z = getFloatAttribute(element, "Z");
        q.w = getFloatAttribute(element, "W");
        memcpy(outData, reinterpret_cast<char*>(&q), getTypeStride(typeName));
    } else if (typeName == "float") {
        XSValue::Status status;
        XSValue* val = XSValue::getActualValue(element->getTextContent(), xercesc::XSValue::DataType::dt_float, status);
        memcpy(outData, reinterpret_cast<char*>(&val->fData.fValue.f_float), getTypeStride(typeName));
    } else if (typeName == "double") {
        XSValue::Status status;
        XSValue* val = XSValue::getActualValue(element->getTextContent(), xercesc::XSValue::DataType::dt_double, status);
        memcpy(outData, reinterpret_cast<char*>(&val->fData.fValue.f_double), getTypeStride(typeName));
    } else if (typeName == "bool") {
        XSValue::Status status;
        XSValue* val = XSValue::getActualValue(element->getTextContent(), xercesc::XSValue::DataType::dt_boolean, status);
        memcpy(outData, reinterpret_cast<char*>(&val->fData.fValue.f_bool), getTypeStride(typeName));
    } else {
        XSValue::DataType dataType = XSValue::getDataType(XSTR(typeName.c_str()));
        if (dataType == XSValue::DataType::dt_string) {
            char* str = XMLString::transcode(element->getTextContent());
            std::string standardString(str);
            new (outData) std::string(str);
            XMLString::release(&str);
            //memcpy(outData, reinterpret_cast<char*>(&standardString), getTypeStride(typeName));
        } else {
            //XSValue::Status status;
            //XSValue* val = XSValue::getActualValue(element->getTextContent(), dataType, status);
            //memcpy(outData, reinterpret_cast<char*>(&val->fData.fValue), getTypeStride(typeName));
            LOG_WARNING("Unknown native data type: %s", typeName.c_str());
        }
    }
}

