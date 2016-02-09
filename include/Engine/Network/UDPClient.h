#ifndef UDPClient_h__
#define UDPClient_h__

#include <boost/asio.hpp>
#include "Network/NetworkClient.h"
//virtual void Connect(std::string address, int port) = 0;
//virtual int Receive(char * data) = 0;
//virtual void Send(Packet & packet) = 0;
//virtual void Disconnect() = 0;

class UDPClient : public NetworkClient
{
public:
    UDPClient();
    ~UDPClient();

    void Connect(std::string playerName, std::string address, int port);
    void Disconnect();
    void Receive(Packet& packet);
    void Send(Packet & packet);
    bool IsSocketAvailable();
private:
    // Assio UDP logic
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::shared_ptr<boost::asio::ip::udp::socket> m_Socket;
    int readBuffer(char* data);
    PacketID m_SendPacketID = 0;
};

#endif