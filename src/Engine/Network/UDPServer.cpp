#include "Network/UDPServer.h"

UDPServer::UDPServer()
{
    m_Socket = std::unique_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 27666)));
}

UDPServer::~UDPServer()
{ }

void UDPServer::readFromClients()
{
    while (m_Socket->available()) {
        try {
            bytesRead = receive(readBuffer);
            m_Address = m_ReceiverEndpoint.address();
            m_Port = m_ReceiverEndpoint.port();
            Packet packet(readBuffer, bytesRead);
            parseMessageType(packet);
        } catch (const std::exception& err) {
            //LOG_ERROR("%i: Read from client crashed %s", m_PacketID, err.what());
        }
    }
}

void UDPServer::parseConnect(Packet& packet)
{
    LOG_INFO("Parsing connections");
    // Check if player is already connected
    if (GetPlayerIDFromEndpoint() != -1) {
        return;
    }
    // Create a new player
    PlayerDefinition pd;
    pd.EntityID = 0; // Overlook this
    pd.Endpoint = m_ReceiverEndpoint;
    pd.Address = m_ReceiverEndpoint.address();
    pd.Port = m_ReceiverEndpoint.port();
    pd.Name = packet.ReadString();
    pd.PacketID = 0;
    pd.StopTime = std::clock();
    m_ConnectedPlayers[m_NextPlayerID++] = pd;
    LOG_INFO("Spectator \"%s\" connected on IP: %s", pd.Name.c_str(), pd.Endpoint.address().to_string().c_str());

    // Send a message to the player that connected
    Packet connnectPacket(MessageType::Connect, pd.PacketID);
    send(connnectPacket);

    // Send notification that a player has connected
    Packet notificationPacket(MessageType::PlayerConnected);
    broadcast(notificationPacket);
}

void UDPServer::send(Packet& packet, PlayerDefinition & playerDefinition)
{
    try {
        int bytesSent = m_Socket->send_to(
            boost::asio::buffer(packet.Data(), packet.Size()),
            playerDefinition.Endpoint,
            0);
        // Network Debug data
        if (isReadingData) {
            m_NetworkData.TotalDataSent += packet.Size();
            m_NetworkData.DataSentThisInterval += packet.Size();
            m_NetworkData.AmountOfMessagesSent++;
        }
    } catch (const boost::system::system_error& e) {
        // TODO: Clean up invalid endpoints out of m_ConnectedPlayers later
        playerDefinition.Endpoint = boost::asio::ip::udp::endpoint();
    }
}
// Send back to endpoint of received packet
void UDPServer::send(Packet & packet)
{
    m_Socket->send_to(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        m_ReceiverEndpoint,
        0);
    if (isReadingData) {
        // Network Debug data
        m_NetworkData.TotalDataSent += packet.Size();
        m_NetworkData.DataSentThisInterval += packet.Size();
    }
}


int UDPServer::receive(char * data)
{
    unsigned int length = m_Socket->receive_from(
        boost::asio::buffer((void*)data
            , BUFFERSIZE)
        , m_ReceiverEndpoint, 0);
    // Network Debug data
    if (isReadingData) {
        m_NetworkData.TotalDataReceived += length;
        m_NetworkData.DataReceivedThisInterval += length;
        m_NetworkData.AmountOfMessagesReceived++;
    }
    return length;
}


