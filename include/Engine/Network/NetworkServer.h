#ifndef NetworkServer_h__
#define NetworkServer_h__
#include <boost/asio.hpp>
#include "Network/Packet.h"
#include "Network/PlayerDefinition.h"
#define BUFFERSIZE 32000
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
    //void handle_accept(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code & error);
    //void parseConnect(Packet & packet);
    //void readFromClients();

    //public:
    //    virtual void Connect(std::string playerName, std::string address, int port) = 0;
    //    virtual void Disconnect() = 0;
    //    virtual Packet Receive() = 0;
    //    virtual void Send(Packet & packet) = 0;
    //protected:
    //    char m_ReadBuffer[BUFFERSIZE] = { 0 };
};

#endif