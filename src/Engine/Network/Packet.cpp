#include "Network/Packet.h"

Packet::Packet(MessageType type, unsigned int& packetID)
{
    m_Data = new char[128];
    // Create message header
    // Add message type
    int messageType = static_cast<int>(type);
    Packet::WritePrimitive<int>(messageType);
    packetID = packetID % 1000; // Packet id modulos
    Packet::WritePrimitive<int>(packetID);
    packetID++;
}


Packet::Packet(char* data, const int sizeOfPacket)
{
    // Create message
    m_Data = new char[sizeOfPacket];
    memcpy(m_Data, data, sizeOfPacket);
    m_Offset = sizeOfPacket;
}

Packet::~Packet()
{
    delete[] m_Data;
}

void Packet::WriteString(std::string str)
{
    // Message, add one extra byte for null terminator
    memcpy(m_Data + m_Offset, str.data(), (str.size() + 1) * sizeof(char));
    m_Offset += (str.size() + 1) * sizeof(char);
}

void Packet::WriteData(char * data, int sizeOfData)
{ 
    if (m_Offset + sizeOfData > 128) {
        LOG_WARNING("Packet::AddData(): Data size in packet exceeded maximum packet size.\n");
    }
    memcpy(m_Data + m_Offset, data, sizeOfData);
    m_Offset += sizeOfData;
}

std::string Packet::ReadString()
{
    std::string returnValue(m_Data + m_ReturnDataOffset);
    if (m_Offset < m_ReturnDataOffset + returnValue.size()){
        LOG_WARNING("packet PopFrontString(): Oh no! You are trying to remove things outside my memory kingdom");
        return "PopFrontString Failed";
    }   
    // +1 for null terminator.
    m_ReturnDataOffset += returnValue.size() + 1;
    return returnValue;
}

char * Packet::ReadData(int SizeOfData)
{
    unsigned int oldReturnDataOffset = m_ReturnDataOffset;
    m_ReturnDataOffset += SizeOfData;
    return (m_Data + oldReturnDataOffset);
}
