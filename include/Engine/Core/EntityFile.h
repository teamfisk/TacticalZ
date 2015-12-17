#ifndef EntityFile_h__
#define EntityFile_h__

#include <stack>
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
    typedef std::function<void(EntityID, EntityID)> OnStartEntityCallback;
    void SetStartEntityCallback(OnStartEntityCallback c) { m_OnStartEntityCallback = c; }
    // @param std::string Type name of the component
    typedef std::function<void(std::string)> OnStartComponentCallback;
    void SetStartComponentCallback(OnStartComponentCallback c) { m_OnStartComponentCallback = c; }
    // @param std::string Field name
    // @param std::string Field type
    typedef std::function<void(std::string, std::string)> OnStartFieldCallback;
    void SetStartFieldCallback(OnStartFieldCallback c) { m_OnStartFieldCallback = c; }
    // @param char* Field data
    typedef std::function<void(char*)> OnStartFieldDataCallback;
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

    EntityFileSAXHandler(const EntityFileHandler* handler, xercesc::SAX2XMLReader* reader)
        : m_Handler(handler)
        , m_Reader(reader)
    {
        // 0 is imaginary world entity
        m_EntityStack.push(0);
    }

    void startElement(const XMLCh* const _uri, const XMLCh* const _localName, const XMLCh* const _qname, const xercesc::Attributes& attrs) override
    {
        std::string name = XS::ToString(_localName);

        if (m_CurrentScope == State::Unknown) {
            if (name == "Entity") {
                m_CurrentScope = State::Entity;
                onStartEntity(attrs);
                return;
            }
            if (name == "EntityRef") {
                onStartEntityRef(attrs);
                return;
            }
        }

        std::string uri = XS::ToString(_uri);
        if (m_CurrentScope == State::Entity) {
            if (uri == "components") {
                m_CurrentScope = State::Component;
                onStartComponent(name);
                return;
            }
        }
        
        if (m_CurrentScope == State::Component) {
            m_CurrentScope = State::ComponentField;
            onStartComponentField(name, attrs);
            return;
        }
    }

    void characters(const XMLCh* const chars, const XMLSize_t length) override
    {
        if (m_CurrentScope == State::ComponentField) {
            char* transcoded = xercesc::XMLString::transcode(chars);
            onFieldData(transcoded);
        }
    }

    void endElement(const XMLCh* const _uri, const XMLCh* const _localName, const XMLCh* const _qname) override
    {
        std::string name = XS::ToString(_localName);
        if (m_CurrentScope == State::Entity) {
            if (name == "Entity") {
                m_CurrentScope = State::Unknown;
                onEndEntity();
                return;
            }
        }
        
        std::string uri = XS::ToString(_uri);
        if (m_CurrentScope == State::Component) {
            //if (uri == "components") {
                m_CurrentScope = State::Entity;
                onEndComponent(name);
                return;
            //}
        }

        if (m_CurrentScope == State::ComponentField) {
            m_CurrentScope = State::Component;
            onEndComponentField(name);
            return;
        }
    }

    void fatalError(const xercesc::SAXParseException& e) 
    {
        XS::ToString s(e.getMessage());
        LOG_ERROR("SAXParseException: %s", ((std::string)s).c_str());
        //throw e;
    }

private:
    const EntityFileHandler* m_Handler;
    xercesc::SAX2XMLReader* m_Reader;
    State m_CurrentScope = State::Unknown;
    unsigned int m_NextEntityID = 1;
    std::stack<EntityID> m_EntityStack;
    std::map<std::string, std::string> m_CurrentAttributes;

    void onStartEntity(const xercesc::Attributes& attrs) 
    {
        EntityID parent = m_EntityStack.top();

        // TODO: Create entity here
        auto xName = attrs.getValue(XS::ToXMLCh("name"));
        std::string name = XS::ToString(xName);
        //LOG_DEBUG("Entity %i (%i): %s", m_NextEntityID, parent, name.c_str());

        if (m_Handler->m_OnStartEntityCallback) {
            m_Handler->m_OnStartEntityCallback(m_NextEntityID, parent);
        }

        m_EntityStack.push(m_NextEntityID);
        m_NextEntityID++;
    } 
    void onEndEntity()
    {
        m_EntityStack.pop();
    }
    void onStartEntityRef(const xercesc::Attributes& attrs)
    {
        std::string path = XS::ToString(attrs.getValue(XS::ToXMLCh("file")));
        
        xercesc::SAX2XMLReader* parser = xercesc::XMLReaderFactory::createXMLReader();
        parser->setContentHandler(this);
        parser->setErrorHandler(this);
        parser->parse(path.c_str());
        delete parser;
    }
    void onStartComponent(std::string name)
    {
        //LOG_DEBUG("    Component: %s", name.c_str());

        if (m_Handler->m_OnStartComponentCallback) {
            m_Handler->m_OnStartComponentCallback(name);
        }
    }
    void onEndComponent(std::string name)
    {
    }
    void onStartComponentField(std::string field, const xercesc::Attributes& attrs) 
    {
        //LOG_DEBUG("        Field: %s", field.c_str());

        for (int i = 0; i < attrs.getLength(); i++) {
            auto name = attrs.getQName(i);
            auto value = attrs.getValue(name);

            //LOG_DEBUG("            %s = %s", (char*)XS::ToString(name), (char*)XS::ToString(value));
            m_CurrentAttributes[XS::ToString(name).operator std::string()] = XS::ToString(value).operator std::string();
        }

        if (m_Handler->m_OnStartFieldCallback) {
            m_Handler->m_OnStartFieldCallback(field, "comment?");
        }
    }
    void onEndComponentField(std::string field)
    {

    }

    void onFieldData(char* data)
    {
        //LOG_DEBUG("            Data: %s", data);

        if (m_Handler->m_OnStartFieldDataCallback) {
            m_Handler->m_OnStartFieldDataCallback(data);
        }

        xercesc::XMLString::release(&data);
    }
};

class EntityFile : public Resource
{
    friend class ResourceManager;
private:
    EntityFile(boost::filesystem::path path);
    ~EntityFile();

public:
    static std::size_t GetTypeStride(std::string typeName);
static void WriteElementData(const xercesc::DOMElement* element, std::string typeName, char* outData)
{
    using namespace xercesc;

    if (typeName == "Vector") {
        glm::vec3 vec;
        vec.x = getFloatAttribute(element, "X");
        vec.y = getFloatAttribute(element, "Y");
        vec.z = getFloatAttribute(element, "Z");
        memcpy(outData, reinterpret_cast<char*>(&vec), GetTypeStride(typeName));
    } else if (typeName == "Color") {
        glm::vec4 vec;
        vec.r = getFloatAttribute(element, "R");
        vec.g = getFloatAttribute(element, "G");
        vec.b = getFloatAttribute(element, "B");
        vec.a = getFloatAttribute(element, "A");
        memcpy(outData, reinterpret_cast<char*>(&vec), GetTypeStride(typeName));
    } else if (typeName == "Quaternion") {
        glm::quat q;
        q.x = getFloatAttribute(element, "X");
        q.y = getFloatAttribute(element, "Y");
        q.z = getFloatAttribute(element, "Z");
        q.w = getFloatAttribute(element, "W");
        memcpy(outData, reinterpret_cast<char*>(&q), GetTypeStride(typeName));
    } else if (typeName == "float") {
        XSValue::Status status;
        XSValue* val = XSValue::getActualValue(element->getTextContent(), xercesc::XSValue::DataType::dt_float, status);
        memcpy(outData, reinterpret_cast<char*>(&val->fData.fValue.f_float), GetTypeStride(typeName));
    } else if (typeName == "double") {
        XSValue::Status status;
        XSValue* val = XSValue::getActualValue(element->getTextContent(), xercesc::XSValue::DataType::dt_double, status);
        memcpy(outData, reinterpret_cast<char*>(&val->fData.fValue.f_double), GetTypeStride(typeName));
    } else if (typeName == "bool") {
        XSValue::Status status;
        XSValue* val = XSValue::getActualValue(element->getTextContent(), xercesc::XSValue::DataType::dt_boolean, status);
        memcpy(outData, reinterpret_cast<char*>(&val->fData.fValue.f_bool), GetTypeStride(typeName));
    } else {
        XSValue::DataType dataType = XSValue::getDataType(XS::ToXMLCh(typeName));
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

    void Parse(const EntityFileHandler* handler);
    //const std::map<std::string, ::ComponentInfo>& ComponentInfo() const { return m_ComponentInfo; }
    //const std::vector<std::string>& EntityReferences() const { return m_EntityReferences; }

    xercesc::XMLGrammarPool* GrammarPool() const { return m_GrammarPool; }

private:
    boost::filesystem::path m_FilePath;
    xercesc::XMLGrammarPool* m_GrammarPool;
    xercesc::SAX2XMLReader* m_SAX2XMLReader;
	//std::map<std::string, ::ComponentInfo> m_ComponentInfo;
    //std::vector<std::string> m_EntityReferences;
static float getFloatAttribute(const xercesc::DOMElement* element, const char* attribute)
{
    using namespace xercesc;
    XSValue::Status status;
    XSValue* val = XSValue::getActualValue(element->getAttribute(XS::ToXMLCh(attribute)), xercesc::XSValue::DataType::dt_double, status);
    if (val == nullptr) {
        LOG_ERROR("Element \"%s\" doesn't have an \"%s\" attribute!", ((std::string)XS::ToString(element->getTagName())).c_str(), attribute);
        return 0.f;
    } else {
        return static_cast<float>(val->fData.fValue.f_double);
    }
}
};

#endif