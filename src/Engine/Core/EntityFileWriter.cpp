#include "Core/EntityFileWriter.h"

#define X(str) XS::ToXMLCh(str)

void EntityFileWriter::WriteWorld(World* world)
{
    WriteEntity(world, 0);
}

void EntityFileWriter::WriteEntity(World* world, EntityID entity)
{
    using namespace xercesc;
    DOMDocument* doc = m_DOMImplementation->createDocument(nullptr, X("Entity"), nullptr);
    DOMElement* root = doc->getDocumentElement();
    root->setAttribute(X("xmlns:xsi"), X("http://www.w3.org/2001/XMLSchema-instance"));
    root->setAttribute(X("xsi:noNamespaceSchemaLocation"), X("../Types/Entity.xsd"));
    root->setAttribute(X("xmlns:c"), X("components"));
    const std::string& name = world->GetName(entity);
    if (!name.empty()) {
        root->setAttribute(X("name"), X(name));
    }
    
    DOMElement* componentsElement = doc->createElement(X("Components"));
    root->appendChild(componentsElement);
    appentEntityComponents(componentsElement, world, entity);

    DOMElement* childrenElement = doc->createElement(X("Children"));
    root->appendChild(childrenElement);
    appendEntityChildren(childrenElement, world, entity);

    try {
        LocalFileFormatTarget* target = new LocalFileFormatTarget(X(m_FilePath.string()));
        DOMLSOutput* output = static_cast<DOMImplementationLS*>(m_DOMImplementation)->createLSOutput();
        output->setByteStream(target);
        m_DOMLSSerializer->write(doc, output);
        delete target;
    } catch (const std::runtime_error& e) {
        LOG_ERROR("Failed to save \"%s\": %s", m_FilePath.c_str(), e.what());
    }

    doc->release();
}

void EntityFileWriter::appendEntityChildren(xercesc::DOMElement* parentElement, const World* world, EntityID entity)
{
    using namespace xercesc;
    DOMDocument* doc = parentElement->getOwnerDocument();

    auto childrenRange = world->GetEntityChildren().equal_range(entity);
    for (auto it = childrenRange.first; it != childrenRange.second; ++it) {
        EntityID childEntity = it->second;

        DOMElement* entityElement = doc->createElement(X("Entity"));
        const std::string& name = world->GetName(childEntity);
        if (!name.empty()) {
            entityElement->setAttribute(X("name"), X(name));
        }
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
        for (auto& kv : c.Info.Fields) {
            std::string fieldName = kv.first;
            auto& field = kv.second;

            // Ignore fields that are equal to the default
            // HACK: This is probably sloooooow, but it's okay.
            if (memcmp(c.Data + field.Offset, c.Info.Defaults.get() + field.Offset, field.Stride) == 0) {
                continue;
            }

            DOMElement* fieldElement = doc->createElement(X(fieldName));
            componentElement->appendChild(fieldElement);

            if (field.Type == "Vector") {
                const glm::vec3& vec = c[fieldName];
                fieldElement->setAttribute(X("X"), X(boost::lexical_cast<std::string>(vec.x)));
                fieldElement->setAttribute(X("Y"), X(boost::lexical_cast<std::string>(vec.y)));
                fieldElement->setAttribute(X("Z"), X(boost::lexical_cast<std::string>(vec.z)));
            } else if (field.Type == "Color") {
                const glm::vec4& vec = c[fieldName];
                fieldElement->setAttribute(X("R"), X(boost::lexical_cast<std::string>(vec.r)));
                fieldElement->setAttribute(X("G"), X(boost::lexical_cast<std::string>(vec.g)));
                fieldElement->setAttribute(X("B"), X(boost::lexical_cast<std::string>(vec.b)));
                fieldElement->setAttribute(X("A"), X(boost::lexical_cast<std::string>(vec.a)));
            } else if (field.Type == "Quaternion") {
                const glm::quat& q = c[fieldName];
                fieldElement->setAttribute(X("X"), X(boost::lexical_cast<std::string>(q.x)));
                fieldElement->setAttribute(X("Y"), X(boost::lexical_cast<std::string>(q.y)));
                fieldElement->setAttribute(X("Z"), X(boost::lexical_cast<std::string>(q.z)));
                fieldElement->setAttribute(X("W"), X(boost::lexical_cast<std::string>(q.w)));
            } else if (field.Type == "int") {
                const int& value = c[fieldName];
                fieldElement->appendChild(doc->createTextNode(X(boost::lexical_cast<std::string>(value))));
            } else if (field.Type == "enum") {
                const ComponentInfo::EnumType& value = c[fieldName];
                auto& enumDef = c.Info.Meta->FieldEnumDefinitions.at(fieldName);
                for (auto& kv : enumDef) {
                    if (kv.second == value) {
                        fieldElement->appendChild(doc->createElement(X(kv.first)));
                        break;
                    }
                }
            } else if (field.Type == "float") {
                const float& value = c[fieldName];
                fieldElement->appendChild(doc->createTextNode(X(boost::lexical_cast<std::string>(value))));
            } else if (field.Type == "double") {
                const double& value = c[fieldName];
                fieldElement->appendChild(doc->createTextNode(X(boost::lexical_cast<std::string>(value))));
            } else if (field.Type == "bool") {
                const bool& value = c[fieldName];
                if (value) {
                    fieldElement->appendChild(doc->createTextNode(X("true")));
                } else {
                    fieldElement->appendChild(doc->createTextNode(X("false")));
                }
            } else if (field.Type == "string") {
                const std::string& value = c[fieldName];
                fieldElement->appendChild(doc->createTextNode(X(value)));
            }
        }
    }
}

