#include "Core/EntityFile.h"

EntityFile::EntityFile(boost::filesystem::path path)
    : m_FilePath(path)
{
    using namespace xercesc;
    XMLPlatformUtils::Initialize();
    m_GrammarPool = new XMLGrammarPoolImpl();
    m_SAX2XMLReader = XMLReaderFactory::createXMLReader(XMLPlatformUtils::fgMemoryManager, m_GrammarPool);
}

EntityFile::~EntityFile()
{
    delete m_SAX2XMLReader;
    delete m_GrammarPool;
    xercesc::XMLPlatformUtils::Terminate();
}

void EntityFile::Parse(const EntityFileHandler* handler) const
{
    using namespace xercesc;

    EntityFileSAXHandler saxHandler(handler, m_SAX2XMLReader);
    setReaderFeatures(m_SAX2XMLReader);
    m_SAX2XMLReader->setContentHandler(&saxHandler);
    m_SAX2XMLReader->setErrorHandler(&saxHandler);
    m_SAX2XMLReader->setDeclarationHandler(&saxHandler);
    m_SAX2XMLReader->parse(m_FilePath.string().c_str());
}

void EntityFile::setReaderFeatures(xercesc::SAX2XMLReader* reader)
{
    using namespace xercesc;
    reader->setFeature(XMLUni::fgXercesCacheGrammarFromParse, true);
    reader->setFeature(XMLUni::fgXercesUseCachedGrammarInParse, true);
    reader->setFeature(XMLUni::fgSAX2CoreValidation, true);
    reader->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);
    reader->setFeature(XMLUni::fgXercesSchema, true);
    reader->setFeature(XMLUni::fgXercesSchemaFullChecking, true);
    reader->setFeature(XMLUni::fgXercesCalculateSrcOfs, true);
}

unsigned int EntityFile::GetTypeStride(std::string typeName)
{
    std::map<std::string, unsigned int> typeStrides{
        { "bool", sizeof(bool) },
        { "int", sizeof(int) },
        { "float", sizeof(float) },
        { "double", sizeof(double) },
        { "string", sizeof(std::string) },
        { "enum", sizeof(ComponentInfo::EnumType) },
        { "Vector", sizeof(glm::vec3) },
        { "Quaternion", sizeof(glm::quat) },
        { "Color", sizeof(glm::vec4) }
    };

    auto it = typeStrides.find(typeName);
    return (it != typeStrides.end()) ? it->second : 0;
}

void EntityFile::WriteAttributeData(char* outData, const ComponentInfo::Field_t& field, const std::map<std::string, std::string>& attributes)
{
    if (field.Type == "Vector") {
        glm::vec3 vec;
        vec.x = boost::lexical_cast<float>(attributes.at("X"));
        vec.y = boost::lexical_cast<float>(attributes.at("Y"));
        vec.z = boost::lexical_cast<float>(attributes.at("Z"));
        memcpy(outData, reinterpret_cast<char*>(&vec), field.Stride);
    } else if (field.Type == "Color") {
        glm::vec4 vec;
        vec.r = boost::lexical_cast<float>(attributes.at("R"));
        vec.g = boost::lexical_cast<float>(attributes.at("G"));
        vec.b = boost::lexical_cast<float>(attributes.at("B"));
        vec.a = boost::lexical_cast<float>(attributes.at("A"));
        memcpy(outData, reinterpret_cast<char*>(&vec), field.Stride);
    } else if (field.Type == "Quaternion") {
        glm::quat q;
        q.x = boost::lexical_cast<float>(attributes.at("X"));
        q.y = boost::lexical_cast<float>(attributes.at("Y"));
        q.z = boost::lexical_cast<float>(attributes.at("Z"));
        q.w = boost::lexical_cast<float>(attributes.at("W"));
        memcpy(outData, reinterpret_cast<char*>(&q), field.Stride);
    } else if (!attributes.empty()) {
        LOG_WARNING("%i attributes not handled by any type conversion!", attributes.size());
    }
}

void EntityFile::WriteValueData(char* outData, const ComponentInfo::Field_t& field, const char* valueData)
{
    // Catch and ignore casting errors so whitespace around string enums won't mess anything up
    try {
        if (field.Type == "int") {
            int value = boost::lexical_cast<int>(valueData);
            memcpy(outData, reinterpret_cast<char*>(&value), field.Stride);
        } else if (field.Type == "enum") {
            ComponentInfo::EnumType value = boost::lexical_cast<ComponentInfo::EnumType>(valueData);
            memcpy(outData, reinterpret_cast<char*>(&value), field.Stride);
        } else if (field.Type == "float") {
            float value = boost::lexical_cast<float>(valueData);
            memcpy(outData, reinterpret_cast<char*>(&value), field.Stride);
        } else if (field.Type == "double") {
            double value = boost::lexical_cast<double>(valueData);
            memcpy(outData, reinterpret_cast<char*>(&value), field.Stride);
        } else if (field.Type == "bool") {
            bool value = (valueData[0] == 't'); // Lazy bool evaluation
            memcpy(outData, reinterpret_cast<char*>(&value), field.Stride);
        } else if (field.Type == "string") {
            new (outData) std::string(valueData);
        } else {
            LOG_WARNING("Unknown value data type: %s", field.Type.c_str());
        }
    } catch (const boost::bad_lexical_cast&) { }
}

EntityFileSAXHandler::EntityFileSAXHandler(const EntityFileHandler* handler, xercesc::SAX2XMLReader* reader) 
    : m_Handler(handler)
    , m_Reader(reader)
{
    // 0 is imaginary base parent
    m_EntityStack.push(0);
    m_StateStack.push(State::Unknown);
}

void EntityFileSAXHandler::startElement(const XMLCh* const _uri, const XMLCh* const _localName, const XMLCh* const _qname, const xercesc::Attributes& attrs)
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

void EntityFileSAXHandler::endElement(const XMLCh* const _uri, const XMLCh* const _localName, const XMLCh* const _qname)
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

    if (m_StateStack.top() == State::ComponentField && name == m_CurrentField) {
        m_StateStack.pop();
        onEndComponentField(name);
        return;
    }
}

void EntityFileSAXHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
    if (m_StateStack.top() == State::ComponentField) {
        char* transcoded = xercesc::XMLString::transcode(chars);
        onFieldData(transcoded);
    }
}

void EntityFileSAXHandler::fatalError(const xercesc::SAXParseException& e)
{
    std::string message = XS::ToString(e.getMessage());
    std::string systemId = XS::ToString(e.getSystemId());
    XMLFileLoc line = e.getLineNumber();
    XMLFileLoc column = e.getColumnNumber();
    LOG_ERROR("SAXParseException\n\tLocation: %s:%i:%i\n\tFatal Error: %s", systemId.c_str(), line, column, message.c_str());
}

void EntityFileSAXHandler::error(const xercesc::SAXParseException& e)
{
    std::string message = XS::ToString(e.getMessage());
    std::string systemId = XS::ToString(e.getSystemId());
    XMLFileLoc line = e.getLineNumber();
    XMLFileLoc column = e.getColumnNumber();
    LOG_ERROR("SAXParseException\n\tLocation: %s:%i:%i\n\tError: %s", systemId.c_str(), line, column, message.c_str());
}

void EntityFileSAXHandler::warning(const xercesc::SAXParseException& e)
{
    std::string message = XS::ToString(e.getMessage());
    std::string systemId = XS::ToString(e.getSystemId());
    XMLFileLoc line = e.getLineNumber();
    XMLFileLoc column = e.getColumnNumber();
    LOG_ERROR("SAXParseException\n\tLocation: %s:%i:%i\n\tWarning: %s", systemId.c_str(), line, column, message.c_str());
}

void EntityFileSAXHandler::onStartEntity(const xercesc::Attributes& attrs)
{
    EntityID parent = m_EntityStack.top();

    if (m_Handler->m_OnStartEntityCallback) {
        std::string name;
        auto xName = attrs.getValue(XS::ToXMLCh("name"));
        if (xName != nullptr) {
            name = XS::ToString(xName);
        }
        m_Handler->m_OnStartEntityCallback(m_NextEntityID, parent, name);
    }

    m_EntityStack.push(m_NextEntityID);
    m_NextEntityID++;
}

void EntityFileSAXHandler::onEndEntity()
{
    m_EntityStack.pop();
}

void EntityFileSAXHandler::onStartEntityRef(const xercesc::Attributes& attrs)
{
    std::string path = XS::ToString(attrs.getValue(XS::ToXMLCh("file")));

    xercesc::SAX2XMLReader* reader = xercesc::XMLReaderFactory::createXMLReader();
    EntityFile::setReaderFeatures(reader);
    reader->setContentHandler(this);
    reader->setErrorHandler(this);
    reader->parse(path.c_str());
    delete reader;
}

void EntityFileSAXHandler::onStartComponentField(const std::string& field, const xercesc::Attributes& attrs)
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

void EntityFileSAXHandler::onEndComponent(const std::string& name) { }

void EntityFileSAXHandler::onStartComponent(const std::string& name)
{
    //LOG_DEBUG("    Component: %s", name.c_str());
    m_CurrentComponent = name;
    if (m_Handler->m_OnStartComponentCallback) {
        m_Handler->m_OnStartComponentCallback(m_EntityStack.top(), name);
    }
}

void EntityFileSAXHandler::onEndComponentField(const std::string& field) { }

void EntityFileSAXHandler::onFieldData(char* data)
{
    //LOG_DEBUG("            Data: %s", data);
    if (m_Handler->m_OnStartFieldDataCallback) {
        m_Handler->m_OnStartFieldDataCallback(m_EntityStack.top(), m_CurrentComponent, m_CurrentField, data);
    }

    xercesc::XMLString::release(&data);
}
