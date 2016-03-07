#include "Network/Packet.h"

Packet::Packet(MessageType type, unsigned int& packetID)
{
    m_Data = new char[m_MaxPacketSize];
    Init(type, packetID, 1, 1);
}

// Create message
Packet::Packet(char* data, const size_t sizeOfPacket)
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
    Init(type, dummy, 1, 1);
}

Packet::~Packet()
{
    delete[] m_Data;
}

void Packet::Init(MessageType type, unsigned int & packetID,
    int sequenceNumber, int totalAmountOfPackets)
{
    m_ReturnDataOffset = 0;
    m_Offset = 0;
    // Create message header
    // allocate memory for size of packet, sequenceNumber and totalPacketesInSequence
    WritePrimitive<int>(0);
    WritePrimitive(sequenceNumber);
    WritePrimitive(totalAmountOfPackets);
    // If you add things before here be sure to change in GetMessageType()
    // Add message type
    int messageType = static_cast<int>(type);
    WritePrimitive<int>(messageType);
    WritePrimitive<int>(packetID);
    packetID++;
    m_HeaderSize = m_Offset;
}

void Packet::WriteString(const std::string& str)
{
    // Message, add one extra byte for null terminator
    size_t sizeOfString = str.size() + 1;
    if (m_Offset + sizeOfString > m_MaxPacketSize) {
        if (m_MaxPacketSize >= 32000) {
            LOG_WARNING("Package::WriteString(): New size is huge %i bytes\n", m_MaxPacketSize*2);
        }
        resizeData();
    }
    memcpy(m_Data + m_Offset, str.data(), str.size() * sizeof(char));
    m_Offset += str.size() * sizeof(char);
    m_Data[m_Offset] = '\0';
    m_Offset += 1;
}

void Packet::WriteData(char * data, int sizeOfData)
{

    if (m_Offset + sizeOfData > m_MaxPacketSize) {
        if (m_MaxPacketSize >= 32000) {
            LOG_WARNING("Package::WriteData(): New size is huge %i bytes\n", m_MaxPacketSize*2);
        }
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

void Packet::ReconstructFromData(char * data, size_t sizeOfData)
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
    int whatisoffset = m_Offset;
    memcpy(m_Data, &m_Offset, sizeof(int));
}

char * Packet::ReadData(int sizeOfData)
{
    if (m_Offset < m_ReturnDataOffset + sizeOfData) {
        //LOG_WARNING("packet ReadData(): Oh no! You are trying to remove things outside my memory kingdom");
        return nullptr;
    }
    size_t oldReturnDataOffset = m_ReturnDataOffset;
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
    memcpy(&messagType, m_Data + 3 * sizeof(int), sizeof(int));
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
