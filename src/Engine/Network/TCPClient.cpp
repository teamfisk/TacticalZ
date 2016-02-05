#include "Network/TCPClient.h"

using namespace boost::asio::ip;

TCPClient::TCPClient(ConfigFile * config) : Client(config)
{
    m_Endpoint = tcp::endpoint(boost::asio::ip::address::from_string(address), port);
    m_Socket = boost::shared_ptr<tcp::socket>(new tcp::socket(m_IOService, m_Endpoint));
    tcp::no_delay option(true);
    m_Socket->set_option(option);
}

TCPClient::~TCPClient()
{

}

void TCPClient::Start(World * world, EventBroker * eventBroker)
{
    Client::Start(world, eventBroker);
}
void TCPClient::connect()
{
    if (!m_IsConnected) {
        boost::system::error_code error = boost::asio::error::host_not_found;
        m_Socket->close();
        m_Socket->connect(m_Endpoint, error);
        LOG_INFO(error.message().c_str());
        if (!error) {
            Packet packet(MessageType::Connect, m_SendPacketID);
            packet.WriteString(m_PlayerName);
            m_StartPingTime = std::clock();
            send(packet);
        }
    }
}
// TODO FIX CRASH TCP CLIENT SEVER DISCONNECTS FIRST
void TCPClient::readFromServer()
{
    while (m_Socket->available()) {
        bytesRead = receive(readBuffer);
        Packet packet(readBuffer, bytesRead);
        parseMessageType(packet);
    }
}

int TCPClient::receive(char * data)
{
    boost::system::error_code error;
    // Read size of packet
    int bytesReceived = m_Socket->read_some(boost
        ::asio::buffer((void*)data, sizeof(int)),
         error);
    int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, data, sizeof(int));

    // Read the rest of the message
    bytesReceived += m_Socket->read_some(boost
        ::asio::buffer((void*)(data + bytesReceived), sizeOfPacket - bytesReceived),
         error);
    // Network Debug data
    if (isReadingData) {
        m_NetworkData.TotalDataReceived += bytesReceived;
        m_NetworkData.DataReceivedThisInterval += bytesReceived;
        m_NetworkData.AmountOfMessagesReceived++;
    }
    if (error) {
        //LOG_ERROR("receive: %s", error.message().c_str());
    }
    return bytesReceived;
}

void TCPClient::send(Packet & packet)
{
    packet.UpdateSize();
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
