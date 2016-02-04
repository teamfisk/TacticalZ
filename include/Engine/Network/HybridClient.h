#ifndef HybridClient_h__
#define HybridClient_h__

#include "Client.h"


class HybridClient : public Client
{
public:
    HybridClient(ConfigFile* config);
    ~HybridClient();
    void Start(World* world, EventBroker* eventBroker);
private:
    // Assio UDP logic
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::socket m_Socket;

    void readFromServer();
    int receive(char * data);
    void send(Packet & packet);
};

#endif