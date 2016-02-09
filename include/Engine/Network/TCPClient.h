#ifndef TCPClient_h__
#define TCPClient_h__

#include <boost/asio.hpp>
#include "NetworkClient.h"

class TCPClient : public NetworkClient
{
public:
    TCPClient();
    ~TCPClient();

    void Connect(std::string playerName, std::string address, int port);
    void Disconnect();
    void Receive(Packet& packet);
    void Send(Packet & packet);
    bool IsSocketAvailable();
private:
    // Assio UDP logic
    //boost::asio::io_service m_IOService;
    //boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    //boost::shared_ptr<boost::asio::ip::udp::socket> m_Socket;
    // Assio TCP logic
    boost::asio::ip::tcp::endpoint m_Endpoint;
    boost::asio::io_service m_IOService;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_Socket;
    int readBuffer(char* data);
    PacketID m_SendPacketID = 0;
    bool m_IsConnected = false;
};

#endif