#include "Network/Package.h"

Package::Package(MessageType type, unsigned int& packageID)
{
    m_Data = new char[128];
    // Create message header
    // Add message type
    int messageType = static_cast<int>(type);
    Package::AddPrimitive<int>(messageType);
    packageID = packageID % 1000; // Packet id modulos
    Package::AddPrimitive<int>(packageID);
    packageID++;
}


Package::Package(char* data, const int sizeOfPackage)
{
    // Create message
    m_Data = new char[sizeOfPackage];
    memcpy(m_Data, data, sizeOfPackage);
    m_Offset = sizeOfPackage;
}

Package::~Package()
{
    delete[] m_Data;
}

void Package::AddString(std::string str)
{
    // Message, add one extra byte for null terminator
    memcpy(m_Data + m_Offset, str.data(), (str.size() + 1) * sizeof(char));
    m_Offset += (str.size() + 1) * sizeof(char);
}

void Package::AddData(char * data, int sizeOfData)
{ 
    if (m_Offset + sizeOfData > 128) {
        LOG_WARNING("Package::AddData(): Data size in package exceeded maximum package size.\n");
    }
    memcpy(m_Data + m_Offset, data, sizeOfData);
    m_Offset += sizeOfData;
}

std::string Package::PopFrontString()
{
    std::string returnValue(m_Data + m_ReturnDataOffset);
    if (m_Offset < m_ReturnDataOffset + returnValue.size()){
        LOG_WARNING("Package PopFrontString(): Oh no! You are trying to remove things outside my memory kingdom");
        return "PopFrontString Failed";
    }   
    // +1 for null terminator.
    m_ReturnDataOffset += returnValue.size() + 1;
    return returnValue;
}

char * Package::PopData(int SizeOfData)
{
    unsigned int oldReturnDataOffset = m_ReturnDataOffset;
    m_ReturnDataOffset += SizeOfData;
    return (m_Data + oldReturnDataOffset);
}
