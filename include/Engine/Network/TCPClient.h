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
    // Assio TCP logic
    boost::asio::ip::tcp::endpoint m_Endpoint;
    boost::asio::io_service m_IOService;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_Socket;
    size_t readBuffer(char* data);
    PacketID m_SendPacketID = 0;
    bool m_IsConnected = false;
};

#endif