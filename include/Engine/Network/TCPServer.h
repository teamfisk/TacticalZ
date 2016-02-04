#ifndef TCPServer_h__
#define TCPServer_h__

#include "Server.h"

class TCPServer : public Server
{
public:
    TCPServer();
    ~TCPServer();

private:
    // TCP logic
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::io_service m_IOService;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    boost::shared_ptr<boost::asio::ip::tcp::socket> lastReceivedSocket;

    void Start(World* world, EventBroker* eventBroker);
    void readFromClients();
    void acceptNewConnections();
    void handle_accept(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code & error);
    void parseDisconnect();
    void parseConnect(Packet & packet);
    ///// Implement method to get which player it was
    void parseClientPing();
    void parsePing();
    void parseOnInputCommand(Packet & packet);
    void parsePlayerTransform(Packet & packet);
    /////////////////////////
    void send(Packet & packet, PlayerDefinition & playerDefinition);
    void send(Packet & packet);
    int receive(char * data, boost::asio::ip::tcp::socket& socket);
    PlayerID GetPlayerIDFromEndpoint(boost::asio::ip::udp::endpoint endpoint);
};

#endif