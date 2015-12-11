#ifndef Package_h__
#define Package_h__

#include <string>
#include "Network/MessageType.h"

// Defines the 
class Package
{
public:
    // arg1: Type of message (Connect, Disconnect...)
    // arg2: PackageID for identifying packet loss.
    Package(MessageType type, unsigned int& packageID);
    ~Package();
    // Add primitive types like int, float, char...
    template<typename T>
    void AddPrimitive(T val)
    {
        memcpy(m_Data + m_Offset, &val, sizeof(T));
        m_Offset += sizeof(T);
    }
    void AddString(std::string str);

    int Size() { return m_Offset; };
    char* Data() { return m_Data; };

private:
    char* m_Data = new char[128];
    int m_Offset = 0;
};

#endif