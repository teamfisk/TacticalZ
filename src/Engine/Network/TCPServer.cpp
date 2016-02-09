#include "Network/TCPServer.h"
using namespace boost::asio::ip;

TCPServer::TCPServer()
{
    acceptor = std::unique_ptr<tcp::acceptor>(new tcp::acceptor(m_IOService, tcp::endpoint(tcp::v4(), 27666)));
}

TCPServer::~TCPServer()
{ }

void TCPServer::readFromClients()
{
    acceptNewConnections();
    for (auto& kv : m_ConnectedPlayers) {
        while (kv.second.TCPSocket->available()) {
            try {
                bytesRead = receive(readBuffer, *kv.second.TCPSocket);
                lastReceivedSocket = kv.second.TCPSocket;
                // Get logic for mother class
                boost::asio::ip::tcp::endpoint remoteEndpoint = kv.second.TCPSocket->remote_endpoint();
                m_Address = remoteEndpoint.address();
                m_Port = remoteEndpoint.port();
                // Recreate packets
                Packet packet(readBuffer, bytesRead);
                parseMessageType(packet);
            } catch (const std::exception& err) {
                //LOG_ERROR("%i: Read from client crashed %s", m_PacketID, err.what());
            }
        }
    }
}

void TCPServer::acceptNewConnections()
{
    boost::shared_ptr<tcp::socket> newSocket = boost::shared_ptr<tcp::socket>(new tcp::socket(m_IOService));
    m_IOService.poll();
    acceptor->async_accept(*newSocket,
        boost::bind(&TCPServer::handle_accept, this, newSocket,
            boost::asio::placeholders::error));
}

void TCPServer::handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error)
{
    if (!error && GetPlayerIDFromEndpoint() == -1) {
        // Add tcp socket to connections
        boost::asio::ip::tcp::no_delay option(true);
        socket->set_option(option);
        PlayerDefinition pd;
        pd.StopTime = std::clock();
        pd.TCPSocket = socket;
        pd.Address = socket.get()->remote_endpoint().address();
        pd.Port = socket.get()->remote_endpoint().port();
        m_ConnectedPlayers[m_NextPlayerID++] = pd;
    }
}

void TCPServer::parseConnect(Packet & packet)
{
    LOG_INFO("Parsing connections");
    // Check if player is already connected
    PlayerID playerID = GetPlayerIDFromEndpoint();
    if (playerID = -1) {
        return;
    }

    // Create a new player
    m_ConnectedPlayers.at(playerID).EntityID = 0; // Overlook this
    m_ConnectedPlayers.at(playerID).Name = packet.ReadString();
    m_ConnectedPlayers.at(playerID).PacketID = 0;
    m_ConnectedPlayers.at(playerID).StopTime = std::clock();
    LOG_INFO("Spectator \"%s\" connected on IP: %s", m_ConnectedPlayers.at(playerID).Name.c_str(), m_ConnectedPlayers.at(playerID).Endpoint.address().to_string().c_str());

    // Send a message to the player that connected
    Packet connnectPacket(MessageType::Connect, m_ConnectedPlayers.at(playerID).PacketID);
    send(connnectPacket);

    // Send notification that a player has connected
    Packet notificationPacket(MessageType::PlayerConnected);
    broadcast(notificationPacket);
}

void TCPServer::send(Packet & packet, PlayerDefinition & playerDefinition)
{
    try {
        packet.UpdateSize();
        int bytesSent = playerDefinition.TCPSocket->send(
            boost::asio::buffer(packet.Data(), packet.Size()),
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

void TCPServer::send(Packet & packet)
{
    packet.UpdateSize();
    lastReceivedSocket->send(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        0);
    if (isReadingData) {
        // Network Debug data
        m_NetworkData.TotalDataSent += packet.Size();
        m_NetworkData.DataSentThisInterval += packet.Size();
    }
}

int TCPServer::receive(char * data, boost::asio::ip::tcp::socket& socket)
{
    boost::system::error_code error;
    // Read size of packet
    int bytesReceived = socket.read_some(boost
        ::asio::buffer((void*)data, sizeof(int)),
        error);
    int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, data, sizeof(int));

    // Read the rest of the message
    bytesReceived += socket.read_some(boost
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