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
    m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address::from_string(address), port);
    m_Socket = boost::shared_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(m_IOService));
    m_Socket->connect(m_ReceiverEndpoint);

    Packet packet(MessageType::Connect, m_SendPacketID);
    packet.WriteString(playerName);
    Send(packet);
}

void UDPClient::Disconnect()
{

}

void UDPClient::Receive(Packet& packet)
{
    int bytesRead = readBuffer(m_ReadBuffer);
    if (bytesRead > 0) { 
        packet.ReconstructFromData(m_ReadBuffer, bytesRead);
    }
}

int UDPClient::readBuffer(char* data)
{
    if (!m_Socket) {
        return 0;
    }
    boost::system::error_code error;
    int bytesReceived = m_Socket->receive_from(boost
        ::asio::buffer((void*)data, BUFFERSIZE),
        m_ReceiverEndpoint,
        0, error);
    if (error) {
        //LOG_ERROR("receive: %s", error.message().c_str());
    }
    return bytesReceived;
}

void UDPClient::Send(Packet& packet)
{
    m_Socket->send_to(boost::asio::buffer(
        packet.Data(),
        packet.Size()),
        m_ReceiverEndpoint, 0);
}

bool UDPClient::IsSocketAvailable()
{
    if (!m_Socket) { 
        return false;
    }
    return m_Socket->available();
}