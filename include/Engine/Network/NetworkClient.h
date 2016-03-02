#ifndef NetworkClient_h__
#define NetworkClient_h__

#include "Network/Packet.h"
#define BUFFERSIZE 64000
typedef unsigned int PlayerID;
typedef unsigned int PacketID;

class NetworkClient
{
public:
    virtual void Connect(std::string playerName, std::string address, int port) = 0;
    virtual void Disconnect() = 0;
    virtual void Receive(Packet& packet) = 0;
    virtual void Send(Packet & packet) = 0;
    virtual bool IsSocketAvailable() = 0;
protected:
    char m_ReadBuffer[BUFFERSIZE] = { 0 };
};

#endif