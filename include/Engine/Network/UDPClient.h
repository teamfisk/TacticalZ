#ifndef UDPClient_h__
#define UDPClient_h__

#include <boost/asio.hpp>
#include "Network/NetworkClient.h"

class UDPClient : public NetworkClient
{
public:
    UDPClient();
    ~UDPClient();

    bool Connect(std::string playerName, std::string address, int port);
    void Disconnect();
    void Receive(Packet& packet);
    void Send(Packet & packet);
    void Broadcast(Packet& packet, int port);
    bool IsSocketAvailable();
private:
    // Assio UDP logic
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::shared_ptr<boost::asio::ip::udp::socket> m_Socket;
    int readBuffer();
    PacketID m_SendPacketID = 0;
};

#endif