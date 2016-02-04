#include "Network/TCPClient.h"

using namespace boost::asio::ip;

TCPClient::TCPClient(ConfigFile * config) : Client(config)
{ 
    m_Endpoint = tcp::endpoint(boost::asio::ip::address::from_string(address), port);
    m_Socket = boost::shared_ptr<tcp::socket>(new tcp::socket(m_IOService));
}

TCPClient::~TCPClient()
{ 

}

void TCPClient::Start(World * world, EventBroker * eventBroker)
{ 
    Client::Start(world, eventBroker);
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error) {
        m_Socket->close();
        m_Socket->connect(m_Endpoint,error);
        LOG_INFO(error.message().c_str());
    }
}

void TCPClient::readFromServer()
{ 

}

int TCPClient::receive(char * data)
{
    return 0;
}

void TCPClient::send(Packet & packet)
{ 
    m_Socket->send(boost::asio::buffer(
        packet.Data(),
        packet.Size()));
    // Network Debug data
    if (isReadingData) {
        m_NetworkData.TotalDataSent += packet.Size();
        m_NetworkData.DataSentThisInterval += packet.Size();
        m_NetworkData.AmountOfMessagesSent++;
    }
}
