#ifndef HybridServer_h__
#define HybridServer_h__

#include "Server.h"
#include <boost/asio/ip/udp.hpp>

class HybridServer : public Server
{
public:
    HybridServer();
    ~HybridServer();
private:
    // UDP logic
    boost::asio::io_service m_IOService;
    std::unique_ptr<boost::asio::ip::udp::socket> m_Socket;

    void readFromClients();
    void parseClientPing();
    void parsePing();
    void parseDisconnect();
    void parseConnect(Packet & packet);
    void parseOnInputCommand(Packet & packet);
    void parsePlayerTransform(Packet & packet);
    void send(Packet & packet, PlayerDefinition & playerDefinition);
    void send(Packet & packet);
    int receive(char * data);
    PlayerID GetPlayerIDFromEndpoint(boost::asio::ip::udp::endpoint endpoint);
};

#endif