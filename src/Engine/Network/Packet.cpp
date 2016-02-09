#include "Network/Packet.h"

Packet::Packet(MessageType type, unsigned int& packetID)
{
    m_Data = new char[m_MaxPacketSize];
    Init(type, packetID);
}

// Create message
Packet::Packet(char* data, const int sizeOfPacket)
{
    // Resize message
    m_MaxPacketSize = sizeOfPacket;
    // Copy data newly allocated memory
    m_Data = new char[sizeOfPacket];
    memcpy(m_Data, data, sizeOfPacket);
    m_Offset = sizeOfPacket;
}

Packet::Packet(MessageType type)
{
    m_Data = new char[m_MaxPacketSize];
    unsigned int dummy = 0;
    Init(type, dummy);
}

Packet::~Packet()
{
    delete[] m_Data;
}

void Packet::Init(MessageType type, unsigned int & packetID)
{
    m_ReturnDataOffset = 0;
    m_Offset = 0;
    // Create message header
    // allocate memory for size of packet(only used in tcp)
    Packet::WritePrimitive<int>(0);
    // Add message type
    int messageType = static_cast<int>(type);
    Packet::WritePrimitive<int>(messageType);
    Packet::WritePrimitive<int>(packetID);
    packetID++;
    m_HeaderSize = m_Offset;
}

void Packet::WriteString(const std::string& str)
{
    // Message, add one extra byte for null terminator
    int sizeOfString = str.size() + 1;
    if (m_Offset + sizeOfString > m_MaxPacketSize) {
        //LOG_WARNING("Package::WriteString(): Data size in packet exceeded maximum package size. New size is %i bytes\n", m_MaxPacketSize*2);
        resizeData();
    }
    memcpy(m_Data + m_Offset, str.data(), sizeOfString * sizeof(char));
    m_Offset += sizeOfString * sizeof(char);
}

void Packet::WriteData(char * data, int sizeOfData)
{

    if (m_Offset + sizeOfData > m_MaxPacketSize) {
        //LOG_WARNING("Packet::WriteData(): Data size in packet exceeded maximum packet size. New size is %i bytes\n", m_MaxPacketSize*2);
        while (m_Offset + sizeOfData > m_MaxPacketSize) {
            resizeData();
        }
    }
    memcpy(m_Data + m_Offset, data, sizeOfData);
    m_Offset += sizeOfData;
}

std::string Packet::ReadString()
{
    std::string returnValue(m_Data + m_ReturnDataOffset);
    if (m_Offset < m_ReturnDataOffset + returnValue.size()) {
        //LOG_WARNING("packet ReadString(): Oh no! You are trying to remove things outside my memory kingdom");
        return "PopFrontString Failed";
    }
    // +1 for null terminator.
    m_ReturnDataOffset += returnValue.size() + 1;
    return returnValue;
}

void Packet::ReconstructFromData(char * data, int sizeOfData)
{
    if (sizeOfData > m_MaxPacketSize) {
        // Delete our data
        delete[] m_Data;
        // Set new max size
        m_MaxPacketSize = sizeOfData;
        m_Data = new char[m_MaxPacketSize];
        // while we resized the old data container.
    }
    memcpy(m_Data, data, sizeOfData);
    m_Offset = sizeOfData;

}

void Packet::UpdateSize()
{
    memcpy(m_Data, &m_Offset, sizeof(int));
}

char * Packet::ReadData(int sizeOfData)
{
    if (m_Offset < m_ReturnDataOffset + sizeOfData) {
        //LOG_WARNING("packet ReadData(): Oh no! You are trying to remove things outside my memory kingdom");
        return nullptr;
    }
    unsigned int oldReturnDataOffset = m_ReturnDataOffset;
    m_ReturnDataOffset += sizeOfData;
    return (m_Data + oldReturnDataOffset);
}

void Packet::ChangePacketID(unsigned int & packetID)
{
    packetID = packetID + 1;
    // Overwrite old PacketID
    memcpy(m_Data + 2*sizeof(int), &packetID, sizeof(int));
}

MessageType Packet::GetMessageType()
{
    MessageType messagType;
    memcpy(&messagType, m_Data + sizeof(int), sizeof(int));
    return messagType;
}

void Packet::resizeData()
{
    resizeData(m_MaxPacketSize * 2);
}

void Packet::resizeData(int size)
{
    // Allocate memory to store our data in
    char* holdData = new char[m_MaxPacketSize];
    // Copy our data to the newly allocated memory
    memcpy(holdData, m_Data, m_Offset);
    // Increase max packet size
    m_MaxPacketSize = size;
    // Delete our data
    delete[] m_Data;
    // Allocate memory
    m_Data = new char[m_MaxPacketSize];
    // Copy our data to new location
    memcpy(m_Data, holdData, m_Offset);
    // Delete the memory allocated to hold our data
    // while we resized the old data container.
    delete[] holdData;
}
