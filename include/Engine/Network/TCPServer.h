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
    void Disconnect();
    int Port() { return m_Port; }
    std::string Address() { return m_Address; }

private:
    // TCP logic
    boost::asio::io_service m_IOService;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    boost::shared_ptr<boost::asio::ip::tcp::socket> lastReceivedSocket;

    void handle_accept(boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
        int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers,
        const boost::system::error_code& error);
    int readBuffer(PlayerDefinition& playerDefinition);
    int GetPort();
    std::string GetAddress();
    int m_Port = 0;
    std::string m_Address = "";
};

#endif