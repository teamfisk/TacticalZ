#ifndef TCPClient_h__
#define TCPClient_h__

#include "Client.h"

class TCPClient : public Client
{
public:
    TCPClient(ConfigFile* config);
    ~TCPClient();
    void Start(World* world, EventBroker* eventBroker);
private:
    // Assio TCP logic
    boost::asio::ip::tcp::endpoint m_Endpoint;
    boost::asio::io_service m_IOService;
    boost::shared_ptr<boost::asio::ip::tcp::socket> m_Socket;

    void readFromServer();
    int receive(char * data);
    void send(Packet & packet);
};

#endif