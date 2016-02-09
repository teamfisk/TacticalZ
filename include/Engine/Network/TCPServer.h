#ifndef TCPServer_h__
#define TCPServer_h__

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <map>
#include "NetworkServer.h"

class TCPServer : public NetworkServer
{
public:
    TCPServer();
    ~TCPServer();
    void AcceptNewConnections(int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers);
    void Receive(Packet & packet, PlayerDefinition & playerDefinition);
    void Send(Packet & packet, PlayerDefinition & playerDefinition);
    void Send(Packet & packet);
private:
    // TCP logic
    boost::asio::io_service m_IOService;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    boost::shared_ptr<boost::asio::ip::tcp::socket> lastReceivedSocket;

    void handle_accept(boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
        int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers,
        const boost::system::error_code& error);
    int readBuffer(char* data, PlayerDefinition& playerDefinition);
};

#endif