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
    boost::asio::io_service m_IOService;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    boost::shared_ptr<boost::asio::ip::tcp::socket> lastReceivedSocket;
    
    void acceptNewConnections();
    void handle_accept(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code & error);
    void readFromClients();
    int receive(char * data, boost::asio::ip::tcp::socket& socket);
    void parseConnect(Packet & packet);
    void send(Packet & packet, PlayerDefinition & playerDefinition);
    void send(Packet & packet);
};

#endif