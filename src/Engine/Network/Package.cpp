#include "Network/Package.h"

Package::Package(MessageType type, unsigned int& packageID)
{
    // Create message header
    // Add message type
    int messageType = static_cast<int>(type);
    Package::AddPrimitive<int>(messageType);
    packageID = packageID % 1000; // Packet id modulos
    Package::AddPrimitive<int>(packageID);
    packageID++;
}


Package::Package(char* data, int sizeOfPackage)
{
    // Create message
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

std::string Package::PopFrontString()
{
    std::string returnValue(m_Data + m_ReturnDataOffset);
    if (m_Offset < m_ReturnDataOffset + returnValue.size()){
        LOG_WARNING("Package PopFrontString(): Oh no! You are trying to remove things outside my memory kingdom");
        return "PopFrontString Failed";
    }   

    m_ReturnDataOffset += returnValue.size();
    return returnValue;
}
