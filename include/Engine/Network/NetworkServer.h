#ifndef NetworkServer_h__
#define NetworkServer_h__
#include <boost/asio.hpp>
#include "Network/Packet.h"
#include "Network/PlayerDefinition.h"
#define BUFFERSIZE 64000
typedef unsigned int PlayerID;
typedef unsigned int PacketID;

class NetworkServer
{
public:
    virtual void AcceptNewConnections(int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers) = 0;
    virtual void Receive(Packet & packet, PlayerDefinition & playerDefinition) = 0;
    virtual void Send(Packet & packet, PlayerDefinition & playerDefinition) = 0;
    virtual void Send(Packet & packet) = 0;
protected:
    char m_ReadBuffer[BUFFERSIZE] = { 0 };
};

#endif