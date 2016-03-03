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
    packet.UpdateSize();
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
    packet.UpdateSize();
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
    packet.UpdateSize();
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
    packet.UpdateSize();
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
    int bytesRead = readBuffer();
    if (bytesRead > 0) {
        packet.ReconstructFromData(m_ReadBuffer, bytesRead);
    }
    playerDefinition.Endpoint = m_ReceiverEndpoint;
}

bool UDPServer::IsSocketAvailable()
{
    return m_Socket->available();
}

int UDPServer::readBuffer()
{
    if (!m_Socket) {
        return 0;
    }
    int addasdasd = m_Socket->available();
    boost::system::error_code error;
    // Read size of packet
     m_Socket->receive_from(boost
        ::asio::buffer((void*)m_ReadBuffer, sizeof(int)),
        m_ReceiverEndpoint, boost::asio::ip::udp::socket::message_peek, error);
    unsigned int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, m_ReadBuffer, sizeof(int));

    if (sizeOfPacket > m_Socket->available()) {
        LOG_WARNING("UDPServer::readBuffer(): We haven't got the whole packet yet.");
        return 0;
    }

    // if the buffer is to small increase the size of it
    if (sizeOfPacket > m_BufferSize) {
        delete[] m_ReadBuffer;
        m_ReadBuffer = new char[sizeOfPacket];
        m_BufferSize = sizeOfPacket;
    }

    // Read the rest of the message
    size_t bytesReceived = m_Socket->receive_from(boost
        ::asio::buffer((void*)(m_ReadBuffer),
            sizeOfPacket),
        m_ReceiverEndpoint, 0, error);
        if (error) {
            //LOG_ERROR("receive: %s", error.message().c_str());
        }
        if (sizeOfPacket > 1000000)
            LOG_WARNING("The packets received are bigger than 1MB");

        return bytesReceived;
}

void UDPServer::AcceptNewConnections(int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers)
{ }