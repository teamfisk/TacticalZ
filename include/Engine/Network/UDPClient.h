#ifndef UDPClient_h__
#define UDPClient_h__

#include "Client.h"


class UDPClient : public Client
{
public:
    UDPClient(ConfigFile* config);
    ~UDPClient();
private:
    // Assio UDP logic
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::ip::udp::socket m_Socket;

    void connect();
    void readFromServer();
    int receive(char * data);
    void send(Packet & packet);
};

#endif