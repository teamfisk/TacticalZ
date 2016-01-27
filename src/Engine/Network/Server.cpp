#include "Network/Server.h"

Server::Server() : m_Socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 27666))
{
    Network::initialize();
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    snapshotInterval = 1000 * config->Get<float>("Networking.SnapshotInterval", 0.05);
    pingIntervalMs = config->Get<float>("Networking.PingIntervalMs", 1000);

}

Server::~Server()
{

}

void Server::Start(World* world, EventBroker* eventBroker)
{
    m_World = world;
    m_EventBroker = eventBroker;
    // Subscribe to events
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &Server::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &Server::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_EEntityDeleted, &Server::OnEntityDeleted);
    EVENT_SUBSCRIBE_MEMBER(m_EComponentDeleted, &Server::OnComponentDeleted);
    LOG_INFO("I am Server. BIP BOP\n");
}

void Server::Update()
{
    readFromClients();
    m_EventBroker->Process<Server>();
    if (isReadingData) {
        Network::Update();
        // Test bit patterns
        //std::bitset<32> bitPattern;
        //bitPattern[0].flip();
        //bitPattern[15].flip();
        //bitPattern[31].flip();
        //std::cout << "bitPattern: " << bitPattern << '\n';
        //unsigned int bitPatternInt = static_cast<unsigned int>(bitPattern.to_ulong());
        //std::cout << "Int pattern: " << bitPatternInt << '\n';
        //std::bitset<32> bitPatternRev(bitPatternInt);
        //std::cout << "bitPatternReverted: " << bitPatternRev << '\n';
    }

}

void Server::readFromClients()
{
    while (m_Socket.available()) {
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
    if (pingIntervalMs < (1000 * (currentTime - previousPingMessage) / (double)CLOCKS_PER_SEC)) {
        sendPing();
        previousPingMessage = currentTime;
    }

    // Time out logic
    if (checkTimeOutInterval < (1000 * (currentTime - timOutTimer) / (double)CLOCKS_PER_SEC)) {
        checkForTimeOuts();
        timOutTimer = currentTime;
    }
}

void Server::parseMessageType(Packet& packet)
{
    PlayerID playerID = GetPlayerIDFromEndpoint(m_ReceiverEndpoint);

    int messageType = packet.ReadPrimitive<int>(); // Read what type off message was sent from server
    // Read packet ID 
    if (playerID != -1) {
        // TODO: Bit pattern stuff!"#!"#
        m_ConnectedPlayers[playerID].LastPacketReceivedID = packet.ReadPrimitive<PacketID>();
    }

    //identifyPacketLoss();
    switch (static_cast<MessageType>(messageType)) {
    case MessageType::Connect:
        parseConnect(packet, playerID);
        break;
    case MessageType::Ping:
        parsePing();
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
    case MessageType::PlayerTransform:
        parsePlayerTransform(packet);
        break;
    default:
        break;
    }
}

int Server::receive(char * data)
{
    unsigned int length = m_Socket.receive_from(
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

void Server::send(PlayerID playerID, Packet& packet)
{
    try {
        int bytesSent = m_Socket.send_to(
            boost::asio::buffer(packet.Data(), packet.Size()),
            m_ConnectedPlayers[playerID].Endpoint,
            0);
        // Network Debug data
        if (isReadingData) {
            m_NetworkData.TotalDataSent += packet.Size();
            m_NetworkData.DataSentThisInterval += packet.Size();
            m_NetworkData.AmountOfMessagesSent++;
        }
    } catch (const boost::system::system_error& e) {
        // TODO: Clean up invalid endpoints out of m_ConnectedPlayers later
        m_ConnectedPlayers[playerID].Endpoint = boost::asio::ip::udp::endpoint();
    }
}

void Server::send(Packet & packet)
{
    m_Socket.send_to(
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

void Server::broadcast(Packet& packet)
{
    for (auto& kv : m_ConnectedPlayers) {
        packet.ChangeHeaderInfo(
            kv.second.PacketID,
            kv.second.LastPacketReceivedID, 
            kv.second.AckBitField
            );
        send(kv.first, packet);
    }
}

// Send snapshot fields
void Server::sendSnapshot()
{
    Packet packet(MessageType::Snapshot);
    addChildrenToPacket(packet, EntityID_Invalid);
    broadcast(packet);
}

void Server::addChildrenToPacket(Packet & packet, EntityID entityID)
{
    auto itPair = m_World->GetChildren(entityID);
    std::unordered_map<std::string, ComponentPool*> worldComponentPools = m_World->GetComponentPools();
    // Loop through every child
    for (auto it = itPair.first; it != itPair.second; it++) {
        EntityID childEntityID = it->second;
        // Write EntityID and parentsID and Entity name
        packet.WritePrimitive(childEntityID);
        packet.WritePrimitive(entityID);
        packet.WriteString(m_World->GetName(childEntityID));
        // Write components to child
        int numberOfComponents = 0;
        for (auto& i : worldComponentPools) {
            if (i.second->KnowsEntity(childEntityID)) {
                numberOfComponents++;
            }
        }
        // Write how many components should be read
        packet.WritePrimitive(numberOfComponents);
        for (auto& i : worldComponentPools) {
            // If the entity exist in the pool
            if (i.second->KnowsEntity(childEntityID)) {
                ComponentWrapper componentWrapper = i.second->GetByEntity(childEntityID);
                // ComponentType
                packet.WriteString(componentWrapper.Info.Name);
                // Loop through fields
                for (auto& componentField : componentWrapper.Info.FieldsInOrder) {
                    ComponentInfo::Field_t fieldInfo = componentWrapper.Info.Fields.at(componentField);
                    if (fieldInfo.Type == "string") {
                        std::string& value = componentWrapper[componentField];
                        packet.WriteString(value);
                    } else {
                        packet.WriteData(componentWrapper.Data + fieldInfo.Offset, fieldInfo.Stride);
                    }
                }
            }
        }
        // Go to to your children
        addChildrenToPacket(packet, childEntityID);
    }
}

void Server::sendPing()
{
    // Prints connected players ping
    //for (int i = 0; i < m_ConnectedPlayers.size(); i++) {
    //    if (m_ConnectedPlayers[i].Endpoint.address() != boost::asio::ip::address()) {
    //        int ping = 1000 * (m_ConnectedPlayers[i].StopTime - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
    //        LOG_INFO("Last packetID received %i: User %i's ping: %i", m_ConnectedPlayers[i].PacketID, i, std::abs(ping));
    //    }
    //}
    // Create ping message
    Packet packet(MessageType::Ping);
    packet.WriteString("Ping from server");
    // Time message
    m_StartPingTime = std::clock();
    // Send message
    broadcast(packet);
}

void Server::checkForTimeOuts()
{
    int startPing = 1000 * m_StartPingTime
        / static_cast<double>(CLOCKS_PER_SEC);

    for (int i = 0; i < m_ConnectedPlayers.size(); i++) {
        if (m_ConnectedPlayers[i].Endpoint.address() != boost::asio::ip::address()) {
            int stopPing = 1000 * m_ConnectedPlayers[i].StopTime /
                static_cast<double>(CLOCKS_PER_SEC);
            if (startPing > stopPing + m_TimeoutMs) {
                LOG_INFO("User %i timed out!", i);
                disconnect(i);
            }
        }
    }
}

void Server::disconnect(PlayerID playerID)
{
    //broadcast("A player disconnected");
    LOG_INFO("User %s disconnected/timed out", m_ConnectedPlayers[playerID].Name.c_str());
    // Remove enteties and stuff (When we can remove entity, remove it and tell clients to remove the copy they have)
    Events::PlayerDisconnected e;
    e.Entity = m_ConnectedPlayers[playerID].EntityID;
    e.PlayerID = playerID;
    m_EventBroker->Publish(e);

    m_ConnectedPlayers.erase(playerID);
}

void Server::parseOnInputCommand(Packet& packet)
{
    PlayerID player = -1;
    // Check which player it was who sent the message
    player = GetPlayerIDFromEndpoint(m_ReceiverEndpoint);
    if (player != -1) {
        while (packet.DataReadSize() < packet.Size()) {
            Events::InputCommand e;
            e.Command = packet.ReadString();
            e.PlayerID = player; // Set correct player id
            e.Value = packet.ReadPrimitive<float>();
            m_EventBroker->Publish(e);
            //LOG_INFO("Server::parseOnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
        }
    }
}

void Server::parseOnPlayerDamage(Packet & packet)
{
    Events::PlayerDamage e;
    e.Damage = packet.ReadPrimitive<double>();
    e.Player = EntityWrapper(m_World, packet.ReadPrimitive<EntityID>());
    m_EventBroker->Publish(e);
    //LOG_DEBUG("Server::parseOnPlayerDamage: Command is %s. Value is %f. PlayerID is %i.", e.DamageAmount, e.PlayerDamagedID, e.TypeOfDamage.c_str());
}

void Server::parseConnect(Packet& packet, PlayerID playerID)
{
    LOG_INFO("Parsing connections");
    // Check if player is already connected
    if (playerID != -1) {
        return;
    }
    // Create a new player
    PlayerDefinition pd;
    pd.EntityID = 0; // Overlook this
    pd.Endpoint = m_ReceiverEndpoint;
    pd.Name = packet.ReadString();
    pd.PacketID = 0;
    pd.StopTime = std::clock();
    Packet connnectPacket(MessageType::Connect, pd.PacketID, pd.LastPacketReceivedID, pd.AckBitField);
    m_ConnectedPlayers[m_NextPlayerID++] = pd;
    LOG_INFO("Spectator \"%s\" connected on IP: %s", pd.Name.c_str(), pd.Endpoint.address().to_string().c_str());

    // Send a message to the player that connected
    send(connnectPacket);

    // Send notification that a player has connected
    Packet notificationPacket(MessageType::PlayerConnected);
    broadcast(notificationPacket);
}

void Server::parseDisconnect()
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

void Server::parseClientPing(PlayerID player)
{
    if (player == -1) {
        return;
    }
    // Return ping
    Packet packet(
        MessageType::Ping, 
        m_ConnectedPlayers[player].PacketID, 
        m_ConnectedPlayers[player].LastPacketReceivedID, 
        m_ConnectedPlayers[player].AckBitField
        );
    packet.WriteString("Ping received");
    send(packet);
}

void Server::parsePing()
{
    for (int i = 0; i < m_ConnectedPlayers.size(); i++) {
        if (m_ConnectedPlayers[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            m_ConnectedPlayers[i].StopTime = std::clock();
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

void Server::kick(PlayerID player)
{
    disconnect(player);
    Packet packet = Packet(MessageType::Kick);
    send(packet);
}

PlayerID Server::GetPlayerIDFromEndpoint(boost::asio::ip::udp::endpoint endpoint)
{
    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.Endpoint.address() == endpoint.address() &&
            kv.second.Endpoint.port() == endpoint.port()) {
            return kv.first;
        }
    }
    return -1;
}

bool Server::OnInputCommand(const Events::InputCommand & e)
{
    //LOG_DEBUG("Server::OnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
    if (e.Command == "LogNetworkBandwidth" && e.Value > 0) {
        if (isReadingData) {
            saveToFile();
        }
        isReadingData = !isReadingData;
        m_SaveDataTimer = std::clock();
    }
    if (e.Command == "KickPlayer" && e.Value > 0) {
        kick(0);
    }

    return true;
}

bool Server::OnPlayerSpawned(const Events::PlayerSpawned & e)
{
    m_ConnectedPlayers[e.PlayerID].EntityID = e.Player.ID;

    Packet packet = Packet(MessageType::OnPlayerSpawned);
    packet.WritePrimitive<EntityID>(e.Player.ID);
    packet.WritePrimitive<EntityID>(e.Spawner.ID);
    // We don't send PlayerID here because it will always be set to -1
    packet.WriteString(m_ConnectedPlayers[e.PlayerID].Name);
    send(e.PlayerID, packet);
    return false;
}

bool Server::OnEntityDeleted(const Events::EntityDeleted & e)
{
    if (!e.Cascaded) {
        Packet packet = Packet(MessageType::EntityDeleted);
        packet.WritePrimitive<EntityID>(e.DeletedEntity);
        broadcast(packet);
    }
    return false;
}

bool Server::OnComponentDeleted(const Events::ComponentDeleted & e)
{
    if (!e.Cascaded) {
        Packet packet = Packet(MessageType::ComponentDeleted);
        packet.WritePrimitive<EntityID>(e.Entity);
        packet.WriteString(e.ComponentType);
        broadcast(packet);
    }
    return false;
}

void Server::parsePlayerTransform(Packet& packet)
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
