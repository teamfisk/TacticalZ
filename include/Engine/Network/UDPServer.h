#ifndef UDPServer_h__
#define UDPServer_h__

#include "NetworkServer.h"
#include <boost/asio/ip/udp.hpp>
#define MAXPACKETSIZE 32000

class UDPServer : public NetworkServer
{
public:
    UDPServer();
    UDPServer(int port);
    ~UDPServer();
    void AcceptNewConnections(int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers);
    void Receive(Packet & packet, PlayerDefinition & playerDefinition);
    void Send(Packet & packet, PlayerDefinition & playerDefinition);
    void Send(Packet & packet); 
    void Send(Packet & packet, boost::asio::ip::udp::endpoint endpoint);
    void Broadcast(Packet & packet, int port);
    bool IsSocketAvailable();
private:
    // UDP logic
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    std::unique_ptr<boost::asio::ip::udp::socket> m_Socket;
    int readBuffer();
};

#endif