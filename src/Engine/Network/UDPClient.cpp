#include "Network/UDPClient.h"

using namespace boost::asio::ip;

UDPClient::UDPClient(ConfigFile * config) : Client(config), m_Socket(m_IOService)
{
    m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address::from_string(address), port);
    m_Socket.connect(m_ReceiverEndpoint);
}

UDPClient::~UDPClient()
{
}

void UDPClient::readFromServer()
{
    while (m_Socket.available()) {
        bytesRead = receive(readBuffer);
        if (bytesRead > 0) {
            Packet packet(readBuffer, bytesRead);
            parseMessageType(packet);
        }
    }
}

int UDPClient::receive(char* data)
{
    boost::system::error_code error;

    int bytesReceived = m_Socket.receive_from(boost
        ::asio::buffer((void*)data, BUFFERSIZE),
        m_ReceiverEndpoint,
        0, error);
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

void UDPClient::send(Packet& packet)
{
    m_Socket.send_to(boost::asio::buffer(
        packet.Data(),
        packet.Size()),
        m_ReceiverEndpoint, 0);
    // Network Debug data
    if (isReadingData) {
        m_NetworkData.TotalDataSent += packet.Size();
        m_NetworkData.DataSentThisInterval += packet.Size();
        m_NetworkData.AmountOfMessagesSent++;
    }
}

void UDPClient::connect()
{
    Packet packet(MessageType::Connect, m_SendPacketID);
    packet.WriteString(m_PlayerName);
    m_StartPingTime = std::clock();
    send(packet);
}