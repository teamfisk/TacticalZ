#include "Core/EntityFilePreprocessor.h"

EntityFilePreprocessor::EntityFilePreprocessor(const EntityFile* entityFile) 
    : m_EntityFile(entityFile)
{
    EntityFileHandler handler;
    handler.SetStartComponentCallback(std::bind(&EntityFilePreprocessor::onStartComponent, this, std::placeholders::_1, std::placeholders::_2));
    m_EntityFile->Parse(&handler);

    //LOG_DEBUG("___ COMPONENT DEFINITIONS ___");
    //for (auto& kv : m_ComponentCounts) {
    //    LOG_DEBUG("%s: %i", kv.first.c_str(), kv.second);
    //}

    parseComponentInfo();

    //for (auto& kv : m_ComponentInfo) {
    //    auto& info = kv.second;
    //    LOG_DEBUG("Component: %s (%s)", info.Name.c_str(), info.Meta->Annotation.c_str());
    //    LOG_DEBUG("Stride: %i", info.Stride);
    //    LOG_DEBUG("Allocation: %i", info.Meta->Allocation);
    //    for (auto& kv : info.Fields) {
    //        auto& field = kv.second;
    //        LOG_DEBUG("\t%i\t%s %s", field.Offset, field.Type.c_str(), kv.first.c_str());
    //    }
    //}

    parseDefaults();
}

void EntityFilePreprocessor::RegisterComponents(World* world)
{
    for (auto& kv : m_ComponentInfo) {
        world->RegisterComponent(kv.second);
    }
}

void EntityFilePreprocessor::onStartComponent(EntityID entity, std::string type)
{
    //LOG_DEBUG("Component: %s", type.c_str());
    m_ComponentCounts[type]++;
}

void EntityFilePreprocessor::parseComponentInfo()
{
    using namespace xercesc;
    EntityFileXMLErrorHandler errorHandler;
    auto grammarPool = m_EntityFile->GrammarPool();
    bool whateverTheFuckThisIs;
    auto xsModel = grammarPool->getXSModel(whateverTheFuckThisIs);

    // Find component xsd element declarations
    // <xs:element name="ComponentName">
    auto topLevelElements = xsModel->getComponents(XSConstants::ELEMENT_DECLARATION);
    for (unsigned int i = 0; i < topLevelElements->getLength(); ++i) {
        auto element = static_cast<XSElementDeclaration*>(topLevelElements->item(i));

        std::string nameSpace = XS::ToString(element->getNamespace());
        if (nameSpace != "components") {
            continue;
        }

        ComponentInfo compInfo;
        compInfo.Meta = std::make_shared<ComponentInfo::Meta_t>();

        // Name
        compInfo.Name = XS::ToString(element->getName());
        bool brk = compInfo.Name == "HiddenForLocalPlayer";
        // Known allocation
        compInfo.Meta->Allocation = m_ComponentCounts[compInfo.Name];
        // Annotation
        auto componentAnnotation = element->getAnnotation();
        if (componentAnnotation != nullptr) {
            compInfo.Meta->Annotation = parseAnnotationXML(componentAnnotation->getAnnotationString());
        } else {
            //LOG_WARNING("Component \"%s\" is missing an annotation!", compInfo.Name.c_str());
        }

        // <xs:complexType>
        auto typeDefinition = element->getTypeDefinition();
        if (typeDefinition == nullptr) {
            continue;
        }
        if (typeDefinition->getTypeCategory() != XSTypeDefinition::COMPLEX_TYPE) {
            LOG_ERROR("Failed to parse component definition for \"%s\": Type definition wasn't COMPLEX_TYPE!", compInfo.Name.c_str());
            continue;
        }
        auto complexTypeDefinition = dynamic_cast<XSComplexTypeDefinition*>(typeDefinition);

        // Attributes
        // <xs:attribute...
        auto attributeUses = complexTypeDefinition->getAttributeUses();
        if (attributeUses != nullptr) {
            for (unsigned int i = 0; i < attributeUses->size(); ++i) {
                auto attributeUse = attributeUses->elementAt(i);
                auto attributeDecl = attributeUse->getAttrDeclaration();
                std::string name = XS::ToString(attributeDecl->getName());

                // HACK: This should never happen since patched Xerces. Run deploy to get the updated DLL.
                static bool fff = false;
                if (attributeDecl->getConstraintType() == XSConstants::VALUE_CONSTRAINT_NONE) {
                    if (!fff) {
                        system("explorer https://imon.nu/deploy.html");
                        fff = true;
                    }
                    continue;
                }

                // Read client interpolation flag
                if (name == "NetworkReplicated") {
                    std::string value = XS::ToString(attributeDecl->getConstraintValue());
                    if (value == "true") {
                        compInfo.Meta->NetworkReplicated = true;
                    }
                }
            }
        }

        // Elements
        // <xs:all>
        auto modelGroupParticle = complexTypeDefinition->getParticle();
        if (modelGroupParticle == nullptr || modelGroupParticle->getTermType() != XSParticle::TERM_MODELGROUP) {
            LOG_ERROR("Failed to parse component definition for \"%s\": Model group particle was null or wasn't TERM_MODELGROUP!", compInfo.Name.c_str());
            continue;
        }
        auto modelGroup = modelGroupParticle->getModelGroupTerm();

        // <xs:element... 
        unsigned int fieldOffset = 0;
        auto particles = modelGroup->getParticles();
        for (unsigned int i = 0; i < particles->size(); ++i) {
            auto particle = particles->elementAt(i);
            if (particle->getTermType() != XSParticle::TERM_ELEMENT) {
                //LOG_ERROR("Failed to parse a field definition in component \"%s\": Particle wasn't TERM_ELEMENT! Skipping.", compInfo.Name.c_str());
                continue;
            }
            auto elementDeclaration = particle->getElementTerm();

            std::string name = XS::ToString(elementDeclaration->getName());
            std::string type = XS::ToString(elementDeclaration->getTypeDefinition()->getName());
            std::string typeNamespace = XS::ToString(elementDeclaration->getTypeDefinition()->getNamespace());
            std::string baseType = XS::ToString(elementDeclaration->getTypeDefinition()->getBaseType()->getName());

            std::string effectiveType = type;
            unsigned int stride = EntityFile::GetTypeStride(type);
            if (stride == 0) {
                stride = EntityFile::GetTypeStride(baseType);
                if (stride == 0) {
                    LOG_WARNING("Field \"%s\" in component \"%s\" uses unexpected field type \"%s\" with base type \"%s\". Skipping.", name.c_str(), compInfo.Name.c_str(), type.c_str(), baseType.c_str());
                    continue;
                }
                effectiveType = baseType;
            }

            // Annotation
            auto fieldAnnotation = elementDeclaration->getAnnotation();
            if (fieldAnnotation != nullptr) {
                compInfo.Meta->FieldAnnotations[name] = parseAnnotationXML(fieldAnnotation->getAnnotationString());
            } else {
                //LOG_WARNING("Component field \"%s.%s\" is missing an annotation!", compInfo.Name.c_str(), name.c_str());
            }

            if (effectiveType == "enum") {
                // Parse potential enum type definition for field type
                if (compInfo.Meta->FieldEnumDefinitions.count(name) == 0) {
                    auto enumTypeDefinition = xsModel->getTypeDefinition(XS::ToXMLCh(type), XS::ToXMLCh("components"));
                    auto xsComplexType = dynamic_cast<XSComplexTypeDefinition*>(enumTypeDefinition);
                    auto xsComplexContent = xsComplexType->getParticle();
                    auto xsExtension = xsComplexContent->getModelGroupTerm();
                    auto xsExtensionParticles = xsExtension->getParticles();
                    auto xsChoice = xsExtensionParticles->elementAt(0)->getModelGroupTerm();
                    auto xsChoiceParticles = xsChoice->getParticles();
                    for (int i = 0; i < xsChoiceParticles->size(); ++i) {
                        auto enumElement = xsChoiceParticles->elementAt(i)->getElementTerm();
                        std::string enumName = XS::ToString(enumElement->getName());
                        std::string enumValue = XS::ToString(enumElement->getConstraintValue());
                        compInfo.Meta->FieldEnumDefinitions[name][enumName] = boost::lexical_cast<ComponentInfo::EnumType>(enumValue);
                        //LOG_DEBUG("ENUM %s = %s", enumName.c_str(), enumValue.c_str());
                    }
                }
            }

            auto& field = compInfo.Fields[name];
            field.Name = name;
            field.Type = effectiveType;
            field.Offset = fieldOffset;
            field.Stride = stride;
            compInfo.FieldsInOrder.push_back(name);
            fieldOffset += stride;
        }

        compInfo.Stride = fieldOffset;
        m_ComponentInfo[compInfo.Name] = compInfo;
    }
}

void EntityFilePreprocessor::parseDefaults()
{
    using namespace xercesc;

    EntityFileXMLErrorHandler errorHandler;

    for (auto& ci : m_ComponentInfo) {
        // Allocate memory for default values
        ci.second.Defaults = std::shared_ptr<char>(new char[ci.second.Stride]);
        memset(ci.second.Defaults.get(), 0, ci.second.Stride);

        std::string componentName = ci.first;

        XercesDOMParser parser(nullptr, XMLPlatformUtils::fgMemoryManager);
        parser.setDoSchema(true);
        parser.setDoNamespaces(true);
        parser.setErrorHandler(&errorHandler);
        parser.setValidationScheme(XercesDOMParser::Val_Always);
        parser.setValidationSchemaFullChecking(true);
        //parser.setDoNamespaces(true);
        //boost::filesystem::path schemaLocation = "Schema/Components/" + componentName + ".xsd";
        //std::string namespaceSchema = schemaLocation.string();
        //parser.setExternalNoNamespaceSchemaLocation("Teamasdasdasdasd.xsd");

        //LOG_DEBUG("Parsing defaults for component %s", componentName.c_str());
        boost::filesystem::path defaultsFile = "Schema/Components/" + componentName + ".xml";

        parser.parse(defaultsFile.string().c_str());
        auto doc = parser.getDocument();
        if (doc == nullptr) {
            LOG_ERROR("%s not found! Skipping.", defaultsFile.string().c_str());
            continue;
        }

        // Find the node in the components namespace matching the component name
        std::string tagName = componentName;
        auto rootNodes = doc->getElementsByTagName(XS::ToXMLCh(tagName));
        if (rootNodes->getLength() == 0) {
            LOG_ERROR("Couldn't find defaults for component \"%s\"! Skipping.", componentName.c_str());
            continue;
        }
        auto componentElement = dynamic_cast<DOMElement*>(rootNodes->item(0));

        // Fill the default value buffer with values
        for (auto& kv : ci.second.Fields) {
            std::string fieldName = kv.first;
            auto& field = kv.second;
            auto fieldNodes = componentElement->getElementsByTagName(XS::ToXMLCh(fieldName));
            auto fieldNode = fieldNodes->item(0);
            if (fieldNode == nullptr) {
                LOG_ERROR("Defaults for component \"%s\" is missing field \"%s\"!", componentName.c_str(), fieldName.c_str());
                continue;
            }
            auto fieldElement = dynamic_cast<DOMElement*>(fieldNode);

            char* data = ci.second.Defaults.get() + field.Offset;

            // Handle potential field attributes
            if (fieldElement->hasAttributes()) {
                std::map<std::string, std::string> attributes;
                auto attributeMap = fieldElement->getAttributes();
                for (int i = 0; i < attributeMap->getLength(); ++i) {
                    auto attribItem = attributeMap->item(i);
                    attributes[XS::ToString(attribItem->getNodeName())] = XS::ToString(attribItem->getNodeValue());
                }
                EntityFile::WriteAttributeData(data, field, attributes);
            }

            auto childNode = fieldElement->getFirstChild();
            if (childNode == nullptr) {
                continue;
            }

            // An enum will either have an element node with a text node inside,
            // or contain a text node directly.
            if (childNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                childNode = childNode->getFirstChild();
            }

            // Handle potential field values
            if (childNode->getNodeType() == DOMNode::TEXT_NODE) {
                char* cstrValue = XMLString::transcode(childNode->getNodeValue());
                EntityFile::WriteValueData(data, field, cstrValue);
                XMLString::release(&cstrValue);
            }
        }
    }
}

std::string EntityFilePreprocessor::parseAnnotationXML(const XMLCh* xml)
{
    using namespace xercesc;

    // Parse annotation XML
    char* annotationString = XMLString::transcode(xml);
    MemBufInputSource annotationInput(reinterpret_cast<const XMLByte*>(annotationString), strlen(annotationString), "MemBuf: Annotation String");
    XercesDOMParser parser(nullptr, XMLPlatformUtils::fgMemoryManager);
    //parser.setErrorHandler(&errorHandler);
    parser.parse(annotationInput);
    XMLString::release(&annotationString);
    auto doc = parser.getDocument();

    // Save documentation string
    auto documentationTags = doc->getElementsByTagName(XS::ToXMLCh("xs:documentation"));
    if (documentationTags->getLength() != 0) {
        auto child = documentationTags->item(0)->getFirstChild();
        if (child != nullptr) {
            return XS::ToString(child->getNodeValue());
        }
    }

    return std::string();
}
