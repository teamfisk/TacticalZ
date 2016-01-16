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

    EntityFileSAXHandler saxHandler(handler, nullptr);
    m_SAX2XMLReader->setFeature(XMLUni::fgXercesCacheGrammarFromParse, true);
    m_SAX2XMLReader->setFeature(XMLUni::fgXercesUseCachedGrammarInParse, true);
    m_SAX2XMLReader->setFeature(XMLUni::fgXercesSchema, true);
    m_SAX2XMLReader->setFeature(XMLUni::fgXercesSchemaFullChecking, true);
    m_SAX2XMLReader->setContentHandler(&saxHandler);
    m_SAX2XMLReader->setErrorHandler(&saxHandler);
    m_SAX2XMLReader->setDeclarationHandler(&saxHandler);
    m_SAX2XMLReader->parse(m_FilePath.string().c_str());
}

std::size_t EntityFile::GetTypeStride(std::string typeName)
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
    if (field.Type == "int") {
        int value = boost::lexical_cast<int>(valueData);
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
}
