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

void EntityFile::Parse(const EntityFileHandler* handler)
{
    using namespace xercesc;

    EntityFileSAXHandler saxHandler(handler, nullptr);
    m_SAX2XMLReader->setFeature(XMLUni::fgXercesCacheGrammarFromParse, true);
    m_SAX2XMLReader->setFeature(XMLUni::fgXercesUseCachedGrammarInParse, true);
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
