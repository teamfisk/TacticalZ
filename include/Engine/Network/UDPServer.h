#ifndef UDPServer_h__
#define UDPServer_h__

#include "Server.h"
#include <boost/asio/ip/udp.hpp>

class UDPServer : public Server
{
public:
    UDPServer();
    ~UDPServer();
private:
    // UDP logic
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    std::unique_ptr<boost::asio::ip::udp::socket> m_Socket;

    void readFromClients();
    int receive(char * data);
    void parseConnect(Packet & packet);
    void send(Packet & packet, PlayerDefinition & playerDefinition);
    void send(Packet & packet);
};

#endif