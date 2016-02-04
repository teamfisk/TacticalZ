#include "Network/HybridServer.h"

HybridServer::HybridServer()
{ 
    m_Socket = std::unique_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 27666)));
   
}

HybridServer::~HybridServer()
{ }


void HybridServer::readFromClients()
{
    while (m_Socket->available()) {
        try {
            bytesRead = receive(readBuffer);
            Packet packet(readBuffer, bytesRead);
            parseMessageType(packet);
        } catch (const std::exception& err) {
            //LOG_ERROR("%i: Read from client crashed %s", m_PacketID, err.what());
        }
    }
    std::clock_t currentTime = std::clock();
    // Send snapshot
    if (snapshotInterval < (1000 * (currentTime - previousSnapshotMessage) / (double)CLOCKS_PER_SEC)) {
        sendSnapshot();
        previousSnapshotMessage = currentTime;
    }

    // Send pings each 
    if (pingIntervalMs < (1000 * (currentTime - previousePingMessage) / (double)CLOCKS_PER_SEC)) {
        sendPing();
        previousePingMessage = currentTime;
    }

    // Time out logic
    if (checkTimeOutInterval < (1000 * (currentTime - timOutTimer) / (double)CLOCKS_PER_SEC)) {
        checkForTimeOuts();
        timOutTimer = currentTime;
    }
}

void HybridServer::parseClientPing()
{
    LOG_INFO("%i: Parsing ping", m_PacketID);
    PlayerID player = GetPlayerIDFromEndpoint(m_ReceiverEndpoint);
    if (player == -1) {
        return;
    }
    // Return ping
    Packet packet(MessageType::Ping, m_ConnectedPlayers[player].PacketID);
    packet.WriteString("Ping received");
    send(packet);
}

void HybridServer::parsePing()
{
    for (int i = 0; i < m_ConnectedPlayers.size(); i++) {
        if (m_ConnectedPlayers[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            m_ConnectedPlayers[i].StopTime = std::clock();
            break;
        }
    }
}

void HybridServer::parseDisconnect()
{
    LOG_INFO("%i: Parsing disconnect", m_PacketID);

    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.Endpoint.address() == m_ReceiverEndpoint.address() &&
            kv.second.Endpoint.port() == m_ReceiverEndpoint.port()) {
            disconnect(kv.first);
            break;
        }
    }
}

void HybridServer::parseConnect(Packet& packet)
{
    LOG_INFO("Parsing connections");
    // Check if player is already connected
    if (GetPlayerIDFromEndpoint(m_ReceiverEndpoint) != -1) {
        return;
    }
    // Create a new player
    PlayerDefinition pd;
    pd.EntityID = 0; // Overlook this
    pd.Endpoint = m_ReceiverEndpoint;
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

void HybridServer::parseOnInputCommand(Packet& packet)
{
    PlayerID player = -1;
    // Check which player it was who sent the message
    player = GetPlayerIDFromEndpoint(m_ReceiverEndpoint);
    if (player != -1) {
        while (packet.DataReadSize() < packet.Size()) {
            Events::InputCommand e;
            e.Command = packet.ReadString();
            e.PlayerID = player; // Set correct player id
            e.Player = EntityWrapper(m_World, m_ConnectedPlayers.at(player).EntityID);
            e.Value = packet.ReadPrimitive<float>();
            m_EventBroker->Publish(e);
            LOG_INFO("Server::parseOnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
        }
    }
}

void HybridServer::parsePlayerTransform(Packet& packet)
{
    glm::vec3 position;
    glm::vec3 orientation;
    position.x = packet.ReadPrimitive<float>();
    position.y = packet.ReadPrimitive<float>();
    position.z = packet.ReadPrimitive<float>();
    orientation.x = packet.ReadPrimitive<float>();
    orientation.y = packet.ReadPrimitive<float>();
    orientation.z = packet.ReadPrimitive<float>();

    PlayerID playerID = GetPlayerIDFromEndpoint(m_ReceiverEndpoint);
    EntityWrapper player(m_World, m_ConnectedPlayers.at(playerID).EntityID);

    if (player.Valid()) {
        player["Transform"]["Position"] = position;
        player["Transform"]["Orientation"] = orientation;
    }
}

void HybridServer::send(Packet& packet, PlayerDefinition & playerDefinition)
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
void HybridServer::send(Packet & packet)
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


int HybridServer::receive(char * data)
{
    unsigned int length = m_Socket->receive_from(
        boost::asio::buffer((void*)data
            , INPUTSIZE)
        , m_ReceiverEndpoint, 0);
    // Network Debug data
    if (isReadingData) {
        m_NetworkData.TotalDataReceived += length;
        m_NetworkData.DataReceivedThisInterval += length;
        m_NetworkData.AmountOfMessagesReceived++;
    }
    return length;
}

PlayerID HybridServer::GetPlayerIDFromEndpoint(boost::asio::ip::udp::endpoint endpoint)
{
    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.Endpoint.address() == endpoint.address() &&
            kv.second.Endpoint.port() == endpoint.port()) {
            return kv.first;
        }
    }
    return -1;
}
