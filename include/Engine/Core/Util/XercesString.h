#ifndef Util_XercesString_h__
#define Util_XercesString_h__

#include <string>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLString.hpp>

namespace XS
{

class ToString
{
public:
    ToString(const XMLCh* const str) { m_AsChar = xercesc::XMLString::transcode(str); }
    ~ToString()
    {
        if (m_AsChar != nullptr) {
            xercesc::XMLString::release(&m_AsChar);
        }
    }

    operator std::string() const { return std::string(m_AsChar); }
private:
    char* m_AsChar = nullptr;
};

class ToXMLCh
{
public:
    ToXMLCh(const std::string str) { m_Transcoded = xercesc::XMLString::transcode(str.c_str()); }
    ToXMLCh(const char* str) { m_Transcoded = xercesc::XMLString::transcode(str); }
    ~ToXMLCh()
    {
        if (m_Transcoded != nullptr) {
            xercesc::XMLString::release(&m_Transcoded);
        }
    }

    operator const XMLCh*() const { return m_Transcoded; }
private:
    XMLCh* m_Transcoded = nullptr;
};

}

#endif
