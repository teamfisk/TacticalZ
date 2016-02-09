#ifndef NetworkServer_h__
#define NetworkServer_h__

#include "Network/Packet.h"
#define BUFFERSIZE 32000
typedef unsigned int PlayerID;
typedef unsigned int PacketID;

class NetworkServer
{
//public:
//    virtual void Connect(std::string playerName, std::string address, int port) = 0;
//    virtual void Disconnect() = 0;
//    virtual Packet Receive() = 0;
//    virtual void Send(Packet & packet) = 0;
//protected:
//    char m_ReadBuffer[BUFFERSIZE] = { 0 };
};

#endif