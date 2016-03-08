#include "Network/UDPClient.h"

using namespace boost::asio::ip;

UDPClient::UDPClient()
{ 
}

UDPClient::~UDPClient()
{ 
}

void UDPClient::Connect(std::string playerName, std::string address, int port)
{
    if (m_Socket) {
        return;
    }
    m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address().from_string(address), port);
    m_Socket = boost::shared_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(m_IOService));
    m_Socket->open(boost::asio::ip::udp::v4());
}

void UDPClient::Disconnect()
{

}

void UDPClient::Receive(Packet& packet)
{
    int bytesRead = readBuffer();
    if (bytesRead > 0) { 
        packet.ReconstructFromData(m_ReadBuffer, bytesRead);
    }
}

int UDPClient::readBuffer()
{
    if (!m_Socket) {
        return 0;
    }
    boost::system::error_code error;
    // Peek header
     m_Socket->receive(boost
        ::asio::buffer((void*)m_ReadBuffer, 5 * sizeof(int)),
         boost::asio::ip::udp::socket::message_peek, error);
    int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, m_ReadBuffer, sizeof(int));
    int packetID = 0;
    memcpy(&packetID, m_ReadBuffer + 4 * sizeof(int), sizeof(int));

    if (sizeOfPacket > m_Socket->available()) {
        LOG_WARNING("UDPClient::readBuffer(): We haven't got the whole packet yet.");
        // return;
    }
    // if the buffer is to small increase the size of it
    if (sizeOfPacket > m_BufferSize) {
        delete[] m_ReadBuffer;
        m_ReadBuffer = new char[sizeOfPacket];
        m_BufferSize = sizeOfPacket;
    }
    // Read the message
    size_t bytesReceived = m_Socket->receive_from(boost
        ::asio::buffer((void*)(m_ReadBuffer),
            sizeOfPacket),
        m_ReceiverEndpoint, 0, error);
    if (error) {
        //LOG_ERROR("receive: %s", error.message().c_str());
    }
    // new char [sizeOfPacket] save in map with packetID as key
    return bytesReceived;
}

void UDPClient::Send(Packet& packet)
{
    packet.UpdateSize();
    m_Socket->send_to(boost::asio::buffer(
        packet.Data(),
        packet.Size()),
        m_ReceiverEndpoint, 0);
} 

void UDPClient::Broadcast(Packet& packet, int port)
{
    packet.UpdateSize();
    m_Socket->set_option(boost::asio::socket_base::broadcast(true));
    m_Socket->send_to(boost::asio::buffer(
        packet.Data(),
        packet.Size()),
        udp::endpoint(boost::asio::ip::address_v4().broadcast(), port)
        , 0);
    m_Socket->set_option(boost::asio::socket_base::broadcast(false));
}

bool UDPClient::IsSocketAvailable()
{
    if (!m_Socket) { 
        return false;
    }
    return m_Socket->available();
}