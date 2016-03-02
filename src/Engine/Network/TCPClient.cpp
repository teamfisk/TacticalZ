#include "Network/TCPClient.h"

using namespace boost::asio::ip;

TCPClient::TCPClient()
{
}

TCPClient::~TCPClient()
{
}

void TCPClient::Connect(std::string playerName, std::string address, int port)
{
    if (m_Socket) {
        if (m_IsConnected) {
            Packet packet(MessageType::Connect, m_SendPacketID);
            packet.WriteString(playerName);
            Send(packet);
            LOG_INFO("Connect message sent again!");
        }
    }
    else if (!m_IsConnected) {
        boost::system::error_code error = boost::asio::error::host_not_found;
        m_Endpoint = tcp::endpoint(boost::asio::ip::address::from_string(address), port);
        m_Socket = std::unique_ptr<tcp::socket>(new tcp::socket(m_IOService));
        m_Socket->connect(m_Endpoint, error);
        tcp::no_delay option(true);
        m_Socket->set_option(option);
        LOG_INFO(error.message().c_str());
        if (!error) {
            m_IsConnected = true;
            Packet packet(MessageType::Connect, m_SendPacketID);
            packet.WriteString(playerName);
            Send(packet);
            LOG_INFO("Connect message sent!");
        }
        // If error
        else {
            m_Socket->close();
            m_Socket = nullptr;
        }
    }
}

void TCPClient::Disconnect()
{ 
    if (!m_IsConnected) {
        return;
    }
    m_Socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    m_Socket->close();
    m_Socket = nullptr;
    m_IsConnected = false;
}

void TCPClient::Receive(Packet& packet)
{
    size_t bytesRead = readBuffer(m_ReadBuffer);
    if (bytesRead > 0) {
        packet.ReconstructFromData(m_ReadBuffer, bytesRead);
    }
}

size_t TCPClient::readBuffer(char* data)
{ 
    if (!m_Socket) {
        return 0;
    }
    boost::system::error_code error;
    // Read size of packet
    size_t bytesReceived = m_Socket->read_some(boost
        ::asio::buffer((void*)data, sizeof(int)),
        error);
    int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, data, sizeof(int));

    // Read the rest of the message
    bytesReceived += m_Socket->read_some(boost
        ::asio::buffer((void*)(data + bytesReceived), sizeOfPacket - bytesReceived),
        error);
    if (error) {
        //LOG_ERROR("receive: %s", error.message().c_str());
    }
    return bytesReceived;
}

void TCPClient::Send(Packet & packet)
{
    if (!m_Socket) {
        LOG_WARNING("TCPClient::Send: Socket is null");
        return;
    }
    packet.UpdateSize();
    boost::system::error_code error;
    m_Socket->send(boost::asio::buffer(
        packet.Data(),
        packet.Size()), 0, error);
}

bool TCPClient::IsSocketAvailable()
{
    if (!m_Socket) {
        return false;
    }
    return m_Socket->available();
}