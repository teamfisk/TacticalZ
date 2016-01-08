#include "Core/EntityFileWriter.h"

#define X(str) XS::ToXMLCh(str)

void EntityFileWriter::WriteWorld(World* world)
{
    using namespace xercesc;
    DOMDocument* doc = m_DOMImplementation->createDocument(nullptr, X("Entity"), nullptr);
    DOMElement* root = doc->getDocumentElement();
    root->setAttribute(X("xmlns:xsi"), X("http://www.w3.org/2001/XMLSchema-instance"));
    root->setAttribute(X("xsi:noNamespaceSchemaLocation"), X("../Types/Entity.xsd"));
    root->setAttribute(X("xmlns:c"), X("components"));
    
    DOMElement* componentsElement = doc->createElement(X("Components"));
    root->appendChild(componentsElement);
    appentEntityComponents(componentsElement, world, 0);

    DOMElement* childrenElement = doc->createElement(X("Children"));
    root->appendChild(childrenElement);
    appendEntityChildren(childrenElement, world, 0);

    XMLFormatTarget* target = new StdOutFormatTarget();
    DOMLSOutput* output = static_cast<DOMImplementationLS*>(m_DOMImplementation)->createLSOutput();
    output->setByteStream(target);
    m_DOMLSSerializer->write(doc, output);
}

void EntityFileWriter::appendEntityChildren(xercesc::DOMElement* parentElement, const World* world, EntityID entity)
{
    using namespace xercesc;
    DOMDocument* doc = parentElement->getOwnerDocument();

    auto childrenRange = world->GetEntityChildren().equal_range(entity);
    for (auto it = childrenRange.first; it != childrenRange.second; ++it) {
        EntityID childEntity = it->second;

        DOMElement* entityElement = doc->createElement(X("Entity"));
        parentElement->appendChild(entityElement);

        DOMElement* componentsElement = doc->createElement(X("Components"));
        entityElement->appendChild(componentsElement);
        appentEntityComponents(componentsElement, world, childEntity);

        DOMElement* childrenElement = doc->createElement(X("Children"));
        entityElement->appendChild(childrenElement);
        appendEntityChildren(childrenElement, world, childEntity);
    }
}

void EntityFileWriter::appentEntityComponents(xercesc::DOMElement* parentElement, const World* world, EntityID entity)
{
    using namespace xercesc;
    DOMDocument* doc = parentElement->getOwnerDocument();
    auto& componentPools = world->GetComponentPools();

    // Step through all component pools to get an entity's components
    // HACK: This is sloooow.
    for (auto& kv : componentPools) {
        const std::string& componentName = kv.first;
        if (!world->HasComponent(entity, componentName)) {
            continue;
        }
        std::string qualifiedComponentName = "c:" + componentName;
        DOMElement* componentElement = doc->createElement(X(qualifiedComponentName));
        parentElement->appendChild(componentElement);
        ComponentWrapper c = kv.second->GetByEntity(entity);
        for (auto& kv : c.Info.FieldTypes) {
            std::string fieldName = kv.first;
            std::string fieldType = kv.second;
            std::size_t fieldOffset = c.Info.FieldOffsets.at(fieldName);

            // Ignore fields that are equal to the default
            // HACK: This is probably sloooooow, but it's okay.
            if (memcmp(c.Data + fieldOffset, c.Info.Defaults.get() + fieldOffset, EntityFile::GetTypeStride(fieldType)) == 0) {
                continue;
            }

            DOMElement* fieldElement = doc->createElement(X(fieldName));
            componentElement->appendChild(fieldElement);

            if (fieldType == "Vector") {
                const glm::vec3& vec = c[fieldName];
                fieldElement->setAttribute(X("X"), X(boost::lexical_cast<std::string>(vec.x)));
                fieldElement->setAttribute(X("Y"), X(boost::lexical_cast<std::string>(vec.y)));
                fieldElement->setAttribute(X("Z"), X(boost::lexical_cast<std::string>(vec.z)));
            } else if (fieldType == "Color") {
                const glm::vec4& vec = c[fieldName];
                fieldElement->setAttribute(X("R"), X(boost::lexical_cast<std::string>(vec.r)));
                fieldElement->setAttribute(X("G"), X(boost::lexical_cast<std::string>(vec.g)));
                fieldElement->setAttribute(X("B"), X(boost::lexical_cast<std::string>(vec.b)));
                fieldElement->setAttribute(X("A"), X(boost::lexical_cast<std::string>(vec.a)));
            } else if (fieldType == "Quaternion") {
                const glm::quat& q = c[fieldName];
                fieldElement->setAttribute(X("X"), X(boost::lexical_cast<std::string>(q.x)));
                fieldElement->setAttribute(X("Y"), X(boost::lexical_cast<std::string>(q.y)));
                fieldElement->setAttribute(X("Z"), X(boost::lexical_cast<std::string>(q.z)));
                fieldElement->setAttribute(X("W"), X(boost::lexical_cast<std::string>(q.w)));
            } else if (fieldType == "int") {
                const int& value = c[fieldName];
                fieldElement->appendChild(doc->createTextNode(X(boost::lexical_cast<std::string>(value))));
            } else if (fieldType == "float") {
                const float& value = c[fieldName];
                fieldElement->appendChild(doc->createTextNode(X(boost::lexical_cast<std::string>(value))));
            } else if (fieldType == "double") {
                const double& value = c[fieldName];
                fieldElement->appendChild(doc->createTextNode(X(boost::lexical_cast<std::string>(value))));
            } else if (fieldType == "bool") {
                const bool& value = c[fieldName];
                if (value) {
                    fieldElement->appendChild(doc->createTextNode(X("true")));
                } else {
                    fieldElement->appendChild(doc->createTextNode(X("false")));
                }
            } else if (fieldType == "string") {
                const std::string& value = c[fieldName];
                fieldElement->appendChild(doc->createTextNode(X(value)));
            }
        }
    }
}

