#include "Network/Server.h"

Server::Server() : m_Socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 13))
{ }

Server::~Server()
{

}


void Server::Start(World* world, EventBroker* eventBroker)
{
    m_World = world;
    m_EventBroker = eventBroker;
    // Subscribe to events
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &Server::OnInputCommand);
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        m_PlayerDefinitions[i].StopTime = std::clock();
    }
    LOG_INFO("I am Server. BIP BOP\n");
}

void Server::Update()
{
    readFromClients();
    m_EventBroker->Process<Server>();
}


void Server::readFromClients()
{
    while (m_Socket.available()) {
        try {
            bytesRead = receive(readBuffer, INPUTSIZE);
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
    if (intervalMs < (1000 * (currentTime - previousePingMessage) / (double)CLOCKS_PER_SEC)) {
        sendPing();
        previousePingMessage = currentTime;
    }

    // Time out logic
    if (checkTimeOutInterval < (1000 * (currentTime - timOutTimer) / (double)CLOCKS_PER_SEC)) {
        checkForTimeOuts();
        timOutTimer = currentTime;
    }
}

void Server::parseMessageType(Packet& packet)
{
    int messageType = packet.ReadPrimitive<int>(); // Read what type off message was sent from server

    // Read packet ID 
    m_PreviousPacketID = m_PacketID;    // Set previous packet id
    m_PacketID = packet.ReadPrimitive<int>(); //Read new packet id
    //identifyPacketLoss();
    switch (static_cast<MessageType>(messageType)) {
    case MessageType::Connect:
        parseConnect(packet);
        break;
    case MessageType::ClientPing:
        //parseClientPing();
        break;
    case MessageType::ServerPing:
        parseServerPing();
        break;
    case MessageType::Message:
        break;
    case MessageType::Snapshot:
        break;
    case MessageType::Disconnect:
        parseDisconnect();
        break;
    case MessageType::OnInputCommand:
        parseOnInputCommand(packet);
        break;
    case MessageType::OnPlayerDamage:
        parseOnPlayerDamage(packet);
        break;
    default:
        break;
    }
}

int Server::receive(char * data, size_t length)
{
    length = m_Socket.receive_from(
        boost::asio::buffer((void*)data
            , length)
        , m_ReceiverEndpoint, 0);
    return length;
}

void Server::send(Packet& packet, int playerID)
{
    int bytesSent = m_Socket.send_to(
        boost::asio::buffer(packet.Data(), packet.Size()),
        m_PlayerDefinitions[playerID].Endpoint,
        0);
}

void Server::send(Packet & packet)
{
    m_Socket.send_to(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        m_ReceiverEndpoint,
        0);
}

void Server::broadcast(Packet& packet)
{
    for (int i = 0; i < MAXCONNECTIONS; ++i) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            packet.ChangePacketID(m_PlayerDefinitions[i].PacketID);
            send(packet, i);
        }
    }
}

// Send snapshot fields
void Server::sendSnapshot()
{
    // Should time this
    std::unordered_map<std::string, ComponentPool*> worldComponentPools = m_World->GetComponentPools();
    for (auto& it : worldComponentPools) {
        Packet packet(MessageType::Snapshot);
        ComponentPool* componentPool = it.second;
        ComponentInfo componentInfo = componentPool->ComponentInfo();
        // Component Type
        packet.WriteString(componentInfo.Name);
        for (auto& componentWrapper : *componentPool) {
            // Components EntityID
            packet.WritePrimitive(componentWrapper.EntityID);
            // Parents EntityID
            packet.WritePrimitive(m_World->GetParent(componentWrapper.EntityID));
            for (auto& componentField : componentWrapper.Info.FieldsInOrder) {
                ComponentInfo::Field_t fieldInfo = componentInfo.Fields.at(componentField);
                if (fieldInfo.Type == "string") {
                    std::string& value = componentWrapper[componentField];
                    packet.WriteString(value);
                } else {
                    packet.WriteData(componentWrapper.Data + fieldInfo.Offset, fieldInfo.Stride);
                }
            }
        }
        broadcast(packet);
    }
}

void Server::sendPing()
{
    // Prints connected players ping
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            int ping = 1000 * (m_PlayerDefinitions[i].StopTime - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
            LOG_INFO("Last packetID received %i: Player %i's ping: %i", m_PlayerDefinitions[i].PacketID, i, ping);
        }
    }
    // Create ping message
    Packet packet(MessageType::ServerPing);
    packet.WriteString("Ping from server");
    // Time message
    m_StartPingTime = std::clock();
    // Send message
    broadcast(packet);
}

void Server::checkForTimeOuts()
{
    int timeOutTimeMs = 5000;
    int startPing = 1000 * m_StartPingTime
        / static_cast<double>(CLOCKS_PER_SEC);

    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            int stopPing = 1000 * m_PlayerDefinitions[i].StopTime /
                static_cast<double>(CLOCKS_PER_SEC);
            if (startPing > stopPing + timeOutTimeMs) {
                LOG_INFO("Player %i timed out!", i);
                disconnect(i);
            }
        }
    }
}

void Server::disconnect(int i)
{
    //broadcast("A player disconnected");
    LOG_INFO("Player %s disconnected/timed out", m_PlayerDefinitions[i].Name.c_str());

    // Remove enteties and stuff
    m_PlayerDefinitions[i].Endpoint = boost::asio::ip::udp::endpoint();
    m_PlayerDefinitions[i].EntityID = -1;
    m_PlayerDefinitions[i].Name = "";
    m_PlayerDefinitions[i].PacketID = 0;
}

void Server::parseOnInputCommand(Packet& packet)
{
    int playerID = -1;
    // Check which player it was who sent the message
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        // if the player is connected set playerID to the correct PlayerID
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()
            && m_PlayerDefinitions[i].Endpoint.port() == m_ReceiverEndpoint.port()) {
            playerID = i;
            break;
        }
    }
    if (playerID != -1) {
        while (packet.DataReadSize() < packet.Size()) {
            Events::InputCommand e;
            e.Command = packet.ReadString();
            e.PlayerID = playerID; // Set correct player id
            e.Value = packet.ReadPrimitive<float>();
            m_EventBroker->Publish(e);
            //LOG_INFO("Server::parseOnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
        }
    }
}

void Server::parseOnPlayerDamage(Packet & packet)
{
    Events::PlayerDamage e;
    e.DamageAmount = packet.ReadPrimitive<double>();
    e.PlayerDamagedID = packet.ReadPrimitive<EntityID>();
    e.TypeOfDamage = packet.ReadString();
    m_EventBroker->Publish(e);
    //LOG_DEBUG("Server::parseOnPlayerDamage: Command is %s. Value is %f. PlayerID is %i.", e.DamageAmount, e.PlayerDamagedID, e.TypeOfDamage.c_str());
}

void Server::parseConnect(Packet& packet)
{
    LOG_INFO("Parsing connections");
    // Check if player is already connected
    if (GetPlayerIDFromEndpoint(m_ReceiverEndpoint) != -1) {
        return;
    }

    // Find an empty spot to put the player in
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == boost::asio::ip::address()) {
            // Create new player
            m_PlayerDefinitions[i].EntityID = createPlayer();
            m_PlayerDefinitions[i].Endpoint = m_ReceiverEndpoint;
            m_PlayerDefinitions[i].Name = packet.ReadString();
            m_PlayerDefinitions[i].PacketID = 0;

            m_PlayerDefinitions[i].StopTime = std::clock();

            LOG_INFO("Player \"%s\" connected on IP: %s", m_PlayerDefinitions[i].Name.c_str(), m_PlayerDefinitions[i].Endpoint.address().to_string().c_str());

            // Send a message to the player that connected
            Packet packet(MessageType::Connect, m_PlayerDefinitions[i].PacketID);
            packet.WritePrimitive<int>(i); // Player ID
            packet.WritePrimitive<EntityID>(m_PlayerDefinitions[i].EntityID); // Entity ID
            send(packet, i);

            // Send notification that a player has connected
            Packet notificationPacket(MessageType::PlayerConnected);
            broadcast(notificationPacket);

            break;
        }
    }
}

void Server::parseDisconnect()
{
    LOG_INFO("%i: Parsing disconnect", m_PacketID);

    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            disconnect(i);
            break;
        }
    }
}

void Server::parseClientPing()
{
    LOG_INFO("%i: Parsing ping", m_PacketID);
    int playerID = GetPlayerIDFromEndpoint(m_ReceiverEndpoint);
    if (playerID == -1) {
        return;
    }
    // Return ping
    Packet packet(MessageType::ClientPing, m_PlayerDefinitions[playerID].PacketID);
    packet.WriteString("Ping received");
    send(packet);
}

void Server::parseServerPing()
{
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            m_PlayerDefinitions[i].StopTime = std::clock();
            break;
        }
    }
}

void Server::identifyPacketLoss()
{
    // if no packets lost, difference should be equal to 1
    int difference = m_PacketID - m_PreviousPacketID;
    if (difference != 1) {
        LOG_INFO("%i Packet(s) were lost...", difference);
    }
}

EntityID Server::createPlayer()
{
    EntityID entityID = m_World->CreateEntity();
    ComponentWrapper transform = m_World->AttachComponent(entityID, "Transform");
    transform["Position"] = glm::vec3(-1.5f, 0.f, 0.f);
    ComponentWrapper model = m_World->AttachComponent(entityID, "Model");
    model["Resource"] = "Models/Core/UnitSphere.obj";
    model["Color"] = glm::vec4(rand()%255 / 255.f, rand()%255 / 255.f, rand() %255 / 255.f, 1.f);
    ComponentWrapper player = m_World->AttachComponent(entityID, "Player");
    return entityID;
}

int Server::GetPlayerIDFromEndpoint(boost::asio::ip::udp::endpoint endpoint)
{
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == endpoint.address() &&
            m_PlayerDefinitions[i].Endpoint.port() == endpoint.port()) {
            return i;
        }
    }
    return -1;
}

bool Server::OnInputCommand(const Events::InputCommand & e)
{
    //LOG_DEBUG("Server::OnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
    return true;
}
