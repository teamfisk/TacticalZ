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
    typedef std::function<void(EntityID, EntityID)> OnStartEntityCallback;
    void SetStartEntityCallback(OnStartEntityCallback c) { m_OnStartEntityCallback = c; }
    // @param EntityID The entity the component corresponds to
    // @param std::string Type name of the component
    typedef std::function<void(EntityID, std::string)> OnStartComponentCallback;
    void SetStartComponentCallback(OnStartComponentCallback c) { m_OnStartComponentCallback = c; }
    // @param EntityID Entity 
    // @param std::string Component name 
    // @param std::string Field name
    // @param std::map<std::string, std::string> Field attribute names and values
    typedef std::function<void(EntityID, std::string, std::string, std::map<std::string, std::string>)> OnStartFieldCallback;
    void SetStartFieldCallback(OnStartFieldCallback c) { m_OnStartFieldCallback = c; }
    // @param EntityID Entity 
    // @param std::string Component name 
    // @param std::string Field name
    // @param char* Field data
    typedef std::function<void(EntityID, std::string, std::string, const char*)> OnStartFieldDataCallback;
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
        m_StateStack.push(State::Unknown);
    }

    void startElement(const XMLCh* const _uri, const XMLCh* const _localName, const XMLCh* const _qname, const xercesc::Attributes& attrs) override
    {
        std::string name = XS::ToString(_localName);

        if (m_StateStack.top() == State::Unknown || m_StateStack.top() == State::Entity) {
            if (name == "Entity") {
                m_StateStack.push(State::Entity);
                onStartEntity(attrs);
                return;
            }
            if (name == "EntityRef") {
                onStartEntityRef(attrs);
                return;
            }
        }

        std::string uri = XS::ToString(_uri);
        if (m_StateStack.top() == State::Entity) {
            if (uri == "components") {
                m_StateStack.push(State::Component);
                onStartComponent(name);
                return;
            }
        }
        
        if (m_StateStack.top() == State::Component) {
            m_StateStack.push(State::ComponentField);
            onStartComponentField(name, attrs);
            return;
        }
    }

    void characters(const XMLCh* const chars, const XMLSize_t length) override
    {
        if (m_StateStack.top() == State::ComponentField) {
            char* transcoded = xercesc::XMLString::transcode(chars);
            onFieldData(transcoded);
        }
    }

    void endElement(const XMLCh* const _uri, const XMLCh* const _localName, const XMLCh* const _qname) override
    {
        std::string name = XS::ToString(_localName);
        if (m_StateStack.top() == State::Entity) {
            if (name == "Entity") {
                m_StateStack.pop();
                onEndEntity();
                return;
            }
        }
        
        std::string uri = XS::ToString(_uri);
        if (m_StateStack.top() == State::Component) {
            //if (uri == "components") {
                m_StateStack.pop();
                onEndComponent(name);
                return;
            //}
        }

        if (m_StateStack.top() == State::ComponentField) {
            m_StateStack.pop();
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
    xercesc::XMLGrammarPool* m_GrammarPool;
    xercesc::SAX2XMLReader* m_Reader;
    //State m_CurrentScope = State::Unknown;
    std::stack<State> m_StateStack;
    unsigned int m_NextEntityID = 1;
    std::stack<EntityID> m_EntityStack;
    std::string m_CurrentComponent;
    std::string m_CurrentField;
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
        m_CurrentComponent = name;
        if (m_Handler->m_OnStartComponentCallback) {
            m_Handler->m_OnStartComponentCallback(m_EntityStack.top(), name);
        }
    }
    void onEndComponent(std::string name) { }
    void onStartComponentField(std::string field, const xercesc::Attributes& attrs) 
    {
        //LOG_DEBUG("        Field: %s", field.c_str());
        m_CurrentField = field;
        m_CurrentAttributes.clear();
        for (int i = 0; i < attrs.getLength(); i++) {
            auto name = attrs.getQName(i);
            auto value = attrs.getValue(name);
            //LOG_DEBUG("            %s = %s", (char*)XS::ToString(name), (char*)XS::ToString(value));
            m_CurrentAttributes[XS::ToString(name).operator std::string()] = XS::ToString(value).operator std::string();
        }

        if (m_Handler->m_OnStartFieldCallback) {
            m_Handler->m_OnStartFieldCallback(m_EntityStack.top(), m_CurrentComponent, field, m_CurrentAttributes);
        }
    }
    void onEndComponentField(std::string field) { }

    void onFieldData(char* data)
    {
        //LOG_DEBUG("            Data: %s", data);

        if (m_Handler->m_OnStartFieldDataCallback) {
            m_Handler->m_OnStartFieldDataCallback(m_EntityStack.top(), m_CurrentComponent, m_CurrentField, data);
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

    static void WriteAttributeData(char* outData, const std::string fieldType, const std::map<std::string, std::string>& attributes)
    {
        if (fieldType == "Vector") {
            glm::vec3 vec;
            vec.x = boost::lexical_cast<float>(attributes.at("X"));
            vec.y = boost::lexical_cast<float>(attributes.at("Y"));
            vec.z = boost::lexical_cast<float>(attributes.at("Z"));
            memcpy(outData, reinterpret_cast<char*>(&vec), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "Color") {
            glm::vec4 vec;
            vec.r = boost::lexical_cast<float>(attributes.at("R"));
            vec.g = boost::lexical_cast<float>(attributes.at("G"));
            vec.b = boost::lexical_cast<float>(attributes.at("B"));
            vec.a = boost::lexical_cast<float>(attributes.at("A"));
            memcpy(outData, reinterpret_cast<char*>(&vec), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "Quaternion") {
            glm::quat q;
            q.x = boost::lexical_cast<float>(attributes.at("X"));
            q.y = boost::lexical_cast<float>(attributes.at("Y"));
            q.z = boost::lexical_cast<float>(attributes.at("Z"));
            q.w = boost::lexical_cast<float>(attributes.at("W"));
            memcpy(outData, reinterpret_cast<char*>(&q), EntityFile::GetTypeStride(fieldType));
        } else if (!attributes.empty()) {
            LOG_WARNING("%i attributes not handled by any type conversion!", attributes.size());
        }
    }

    static void WriteValueData(char* outData, const std::string fieldType, const char* valueData)
    {
        if (fieldType == "int") {
            int value = boost::lexical_cast<int>(valueData);
            memcpy(outData, reinterpret_cast<char*>(&value), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "float") {
            float value = boost::lexical_cast<float>(valueData);
            memcpy(outData, reinterpret_cast<char*>(&value), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "double") {
            double value = boost::lexical_cast<double>(valueData);
            memcpy(outData, reinterpret_cast<char*>(&value), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "bool") {
            bool value = (valueData[0] == 't'); // Lazy bool evaluation
            memcpy(outData, reinterpret_cast<char*>(&value), EntityFile::GetTypeStride(fieldType));
        } else if (fieldType == "string") {
            new (outData) std::string(valueData);
        } else {
            LOG_WARNING("Unknown value data type: %s", fieldType.c_str());
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