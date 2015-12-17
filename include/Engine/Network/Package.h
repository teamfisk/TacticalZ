#ifndef Package_h__
#define Package_h__

#include <string>
#include "Network/MessageType.h"
#include "Core/Util/Logging.h"

// Defines the 
class Package
{
public:
    // arg1: Type of message (Connect, Disconnect...)
    // arg2: PackageID for identifying packet loss.
    Package(MessageType type, unsigned int& packageID);
    // Used to create package from already existing data buffer.
    Package(char* data, const int sizeOfPackage);
    
    ~Package();
    // Add primitive types like int, float, char...
    template<typename T>
    void AddPrimitive(T val)
    {
        memcpy(m_Data + m_Offset, &val, sizeof(T));
        m_Offset += sizeof(T);
    }
    // Pops the first element as if it was a primitive.
    template<typename T>
    T PopFrontPrimitive()
    {
        if (m_Offset < m_ReturnDataOffset + sizeof(T)) {
            LOG_WARNING("Package PopFrontPrimitive(): You are trying to remove more than what exists in this package!");
            return -1;
        }
        T returnValue;
        memcpy(&returnValue, m_Data + m_ReturnDataOffset, sizeof(T));
        m_ReturnDataOffset += sizeof(T);
        return returnValue;
    }
    // Add a string to the message
    void AddString(std::string str);
    // Add data to the message
    void AddData(char* data, int sizeOfData);
    // Pops the first element as if it was a string.
    std::string PopFrontString();
    char* PopData(int SizeOfData);

    int Size() { return m_Offset; };
    char* Data() { return m_Data; };

private:
    char* m_Data;
    unsigned int m_ReturnDataOffset = 0;
    int m_Offset = 0;
};

#endif