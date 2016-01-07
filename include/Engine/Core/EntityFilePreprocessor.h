#include <xercesc/framework/psvi/XSElementDeclaration.hpp>
#include <xercesc/framework/psvi/XSComplexTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSParticle.hpp>
#include <xercesc/framework/psvi/XSModelGroup.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/framework/psvi/XSValue.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include "Util/XercesString.h"
#include "ResourceManager.h"
#include "World.h"
#include "EntityFile.h"

class EntityFilePreprocessor
{
public:
    EntityFilePreprocessor(std::string path)
        : m_FilePath(path)
    {
        m_EntityFile = ResourceManager::Load<EntityFile>(path);
        EntityFileHandler handler;
		handler.SetStartComponentCallback(std::bind(&EntityFilePreprocessor::onStartComponent, this, std::placeholders::_1, std::placeholders::_2));
        m_EntityFile->Parse(&handler);

        LOG_DEBUG("___ COMPONENT DEFINITIONS ___");
        for (auto& kv : m_ComponentCounts) {
            LOG_DEBUG("%s: %i", kv.first.c_str(), kv.second);
        }

        parseComponentInfo();

        for (auto& kv : m_ComponentInfo) {
            auto& info = kv.second;
            LOG_DEBUG("Component: %s (%s)", info.Name.c_str(), info.Meta.Annotation.c_str());
            LOG_DEBUG("Stride: %i", info.Meta.Stride);
            LOG_DEBUG("Allocation: %i", info.Meta.Allocation);
            for (auto& kv : info.FieldTypes) {
                LOG_DEBUG("\t%i\t%s %s", info.FieldOffsets[kv.first], kv.second.c_str(), kv.first.c_str());
            }
        }

        parseDefaults();
    }

    void RegisterComponents(World* world)
    {
        for (auto& kv : m_ComponentInfo) {
            world->RegisterComponent(kv.second);
        }
    }

private:
    std::string m_FilePath;
    EntityFile* m_EntityFile;
    std::map<std::string, unsigned int> m_ComponentCounts;
	std::map<std::string, ComponentInfo> m_ComponentInfo;

	void onStartComponent(EntityID entity, std::string type)
	{
		//LOG_DEBUG("Component: %s", type.c_str());
        m_ComponentCounts[type]++;
	}

    void parseComponentInfo()
    {
        using namespace xercesc;
        auto grammarPool = m_EntityFile->GrammarPool();
        bool whateverTheFuckThisIs;
        auto xsModel = grammarPool->getXSModel(whateverTheFuckThisIs);

        // Find component xsd element declarations
        std::cout << "Enumerating components..." << std::endl;
        // <xs:element name="ComponentName">
        auto topLevelElements = xsModel->getComponents(XSConstants::ELEMENT_DECLARATION);
        for (unsigned int i = 0; i < topLevelElements->getLength(); ++i) {
            auto element = static_cast<XSElementDeclaration*>(topLevelElements->item(i));

            std::string nameSpace = XS::ToString(element->getNamespace());
            if (nameSpace != "components") {
                continue;
            }

            ComponentInfo compInfo;

            // Name
            compInfo.Name = XS::ToString(element->getName());
            // Known allocation
            compInfo.Meta.Allocation = m_ComponentCounts[compInfo.Name];
            // Annotation
            auto componentAnnotation = element->getAnnotation();
            if (componentAnnotation != nullptr) {
                // Parse annotation XML
                char* annotationString = XMLString::transcode(componentAnnotation->getAnnotationString());
                MemBufInputSource annotationInput(reinterpret_cast<const XMLByte*>(annotationString), strlen(annotationString), "MemBuf: Annotation String");
                XercesDOMParser parser(nullptr, XMLPlatformUtils::fgMemoryManager, grammarPool);
                //parser.setErrorHandler(m_ErrorHandler);
                parser.parse(annotationInput);
                XMLString::release(&annotationString);
                auto doc = parser.getDocument();

                // TODO: Add allocation estimations from external file on map-to-map basis
                // Add allocation estimation(s)
                //auto allocationTags = doc->getElementsByTagName(XSTR("meta:allocation"));
                //for (int i = 0; i < allocationTags->getLength(); ++i) {
                //    auto allocation = dynamic_cast<DOMElement*>(allocationTags->item(i));
                //    auto child = allocation->getFirstChild();
                //    if (child == nullptr) {
                //        continue;
                //    }

                //    XSValue::Status status;
                //    XSValue* val = XSValue::getActualValue(child->getNodeValue(), XSValue::dt_integer, status);
                //    compInfo.Meta.Allocation += val->fData.fValue.f_int;
                //}

                // Save documentation string
                auto documentationTags = doc->getElementsByTagName(XS::ToXMLCh("xs:documentation"));
                if (documentationTags->getLength() != 0) {
                    auto child = documentationTags->item(0)->getFirstChild();
                    if (child != nullptr) {
                        compInfo.Meta.Annotation = XS::ToString(child->getNodeValue());
                    }
                }
            } else {
                LOG_WARNING("Component is missing an annotation!");
            }

            // <xs:complexType>
            auto typeDefinition = element->getTypeDefinition();
            if (typeDefinition->getTypeCategory() != XSTypeDefinition::COMPLEX_TYPE) {
                LOG_ERROR("Type definition wasn't COMPLEX_TYPE! Skipping.");
                continue;
            }
            auto complexTypeDefinition = dynamic_cast<XSComplexTypeDefinition*>(typeDefinition);

            // <xs:all>
            auto modelGroupParticle = complexTypeDefinition->getParticle();
            if (modelGroupParticle->getTermType() != XSParticle::TERM_MODELGROUP) {
                LOG_ERROR("Model group particle wasn't TERM_MODELGROUP! Skipping.");
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
                    LOG_ERROR("Particle wasn't TERM_ELEMENT! Skipping.");
                    continue;
                }
                auto elementDeclaration = particle->getElementTerm();

                std::string name = XS::ToString(elementDeclaration->getName());
                std::string type = XS::ToString(elementDeclaration->getTypeDefinition()->getName());

                size_t stride = EntityFile::GetTypeStride(type);
                if (stride == 0) {
                    std::cout << "Warning: Field \"" << name << "\" in component \"" << compInfo.Name << "\" uses unexpected field type \"" << type << "\". Skipping." << std::endl;
                    continue;
                }

                compInfo.FieldTypes[name] = type;
                compInfo.FieldOffsets[name] = fieldOffset;
                fieldOffset += stride;
            }

            compInfo.Meta.Stride = fieldOffset;
            m_ComponentInfo[compInfo.Name] = compInfo;
        }
    }

void parseDefaults()
{
    using namespace xercesc;

    for (auto& ci : m_ComponentInfo) {
        // Allocate memory for default values
        ci.second.Defaults = std::shared_ptr<char>(new char[ci.second.Meta.Stride]);
        memset(ci.second.Defaults.get(), 0, ci.second.Meta.Stride);

        XercesDOMParser parser(nullptr, XMLPlatformUtils::fgMemoryManager);
        //parser.setErrorHandler(m_ErrorHandler);

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
        auto rootNodes = doc->getElementsByTagName(XS::ToXMLCh(tagName));
        if (rootNodes->getLength() == 0) {
            LOG_ERROR("Couldn't find defaults for component \"%s\"! Skipping.", componentName.c_str());
            continue;
        }
        auto componentElement = dynamic_cast<DOMElement*>(rootNodes->item(0));
       
        // Fill the default value buffer with values
        for (auto& field : ci.second.FieldOffsets) {
            std::string fieldName = field.first;
            auto fieldNodes = componentElement->getElementsByTagName(XS::ToXMLCh(fieldName));
            auto fieldNode = fieldNodes->item(0);
            if (fieldNode == nullptr) {
                LOG_ERROR("Defaults for component \"%s\" is missing field \"%s\"!", componentName.c_str(), fieldName.c_str());
                continue;
            }
            auto fieldElement = dynamic_cast<DOMElement*>(fieldNode);

            std::string fieldType = ci.second.FieldTypes.at(fieldName);
            unsigned int fieldOffset = ci.second.FieldOffsets.at(fieldName);
            EntityFile::WriteElementData(fieldElement, fieldType, ci.second.Defaults.get() + fieldOffset);
        }
    }
}
};