#ifndef Packet_h__
#define Packet_h__

#include <string>
#include "Network/MessageType.h"
#include "Core/Util/Logging.h"

// Defines the 
class Packet
{
public:
    // arg1: Type of message (Connect, Disconnect...)
    // arg2: PacketID for identifying packet loss.
    Packet(MessageType type, unsigned int& packetID);
    // Used to create packet from already existing data buffer.
    Packet(char* data, const size_t sizeOfPacket);
    Packet(MessageType type);
    ~Packet();
    void Init(MessageType type, unsigned int& packetID);

    // Add primitive types like int, float, char...
    template<typename T>
    void WritePrimitive(T val)
    {
        // Check if we are trying to add more than the package can fit.
        if (m_MaxPacketSize < m_Offset + sizeof(T)) {
            //LOG_WARNING("Packet AddPrimitive(): You are trying to add more than we have allocated for! New size is %i bytes\n", m_MaxPacketSize*2);
            resizeData();
        }
        memcpy(m_Data + m_Offset, &val, sizeof(T));
        m_Offset += sizeof(T);
    }
    // Pops the first element as if it was a primitive.
    template<typename T>
    T ReadPrimitive()
    {
        if (m_Offset < m_ReturnDataOffset + sizeof(T)) {
            //LOG_WARNING("Packet PopFrontPrimitive(): You are trying to remove more than what exists in this packet!");
            return -1;
        }
        T returnValue;
        memcpy(&returnValue, m_Data + m_ReturnDataOffset, sizeof(T));
        m_ReturnDataOffset += sizeof(T);
        return returnValue;
    }
    // Add a string to the message
    void WriteString(const std::string& str);
    // Add data to the message
    void WriteData(char* data, int sizeOfData);
    // Pops the first element as if it was a string.
    std::string ReadString();
    // Construct a packet
    void ReconstructFromData(char* data, size_t SizeOfData);
    // Update size of packet variable in header
    void UpdateSize();
    char* ReadData(int SizeOfData);
    void ChangePacketID(unsigned int& packetID);
    size_t Size() { return m_Offset; };
    char* Data() { return m_Data; };
    MessageType GetMessageType();
    size_t DataReadSize() { return m_ReturnDataOffset; }
    size_t MaxSize() { return m_MaxPacketSize; }
    size_t HeaderSize() { return m_HeaderSize; }

private:
    char* m_Data;
    size_t m_ReturnDataOffset = 0;
    size_t m_Offset = 0;
    size_t m_MaxPacketSize = 512;
    size_t m_HeaderSize = 0;
    void resizeData();
    void resizeData(int size);
};

#endif