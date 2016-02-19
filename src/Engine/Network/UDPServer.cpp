#include "Network/UDPServer.h"

UDPServer::UDPServer()
{
    m_Socket = std::unique_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 27666)));
}

UDPServer::UDPServer(int port)
{
    m_Socket = std::unique_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)));
}

UDPServer::~UDPServer()
{ }

void UDPServer::Send(Packet& packet, PlayerDefinition & playerDefinition)
{
    try {
        int bytesSent = m_Socket->send_to(
            boost::asio::buffer(packet.Data(), packet.Size()),
            playerDefinition.Endpoint,
            0);
    } catch (const boost::system::system_error& e) {
        // TODO: Clean up invalid endpoints out of m_ConnectedPlayers later
        playerDefinition.Endpoint = boost::asio::ip::udp::endpoint();
    }
}
// Send back to endpoint of received packet
void UDPServer::Send(Packet & packet)
{
    m_Socket->send_to(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        m_ReceiverEndpoint,
        0);
}

// Broadcasting respond specific logic
void UDPServer::Send(Packet & packet, boost::asio::ip::udp::endpoint endpoint)
{
    m_Socket->send_to(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        endpoint,
        0);
}

// Broadcasting
void UDPServer::Broadcast(Packet & packet, int port)
{
    m_Socket->set_option(boost::asio::socket_base::broadcast(true));
    m_Socket->send_to(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4().broadcast(),port),
        0);
    m_Socket->set_option(boost::asio::socket_base::broadcast(false));
}

void UDPServer::Receive(Packet & packet, PlayerDefinition & playerDefinition)
{
    int bytesRead = readBuffer(m_ReadBuffer);
    if (bytesRead > 0) {
        packet.ReconstructFromData(m_ReadBuffer, bytesRead);
    }
    playerDefinition.Endpoint = m_ReceiverEndpoint;
}

bool UDPServer::IsSocketAvailable()
{
    return m_Socket->available();
}

int UDPServer::readBuffer(char* data)
{
    boost::system::error_code error = boost::asio::error::host_not_found;
    unsigned int length = m_Socket->receive_from(
        boost::asio::buffer((void*)data
            , BUFFERSIZE)
        , m_ReceiverEndpoint, 0, error);
    if (error) {
        LOG_WARNING(error.message().c_str());
    }
    return length;
}

void UDPServer::AcceptNewConnections(int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers)
{ }