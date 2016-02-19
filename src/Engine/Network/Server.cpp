#include "Network/Server.h"

Server::Server(World* world, EventBroker* eventBroker, int port)
    : Network(world, eventBroker)
{
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    snapshotInterval = 1000 * config->Get<float>("Networking.SnapshotInterval", 0.05f);
    pingIntervalMs = config->Get<float>("Networking.PingIntervalMs", 1000);
    // Subscribe to events
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &Server::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &Server::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_EEntityDeleted, &Server::OnEntityDeleted);
    EVENT_SUBSCRIBE_MEMBER(m_EComponentDeleted, &Server::OnComponentDeleted);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &Server::OnPlayerDamage);

    // Bind
    if (port == 0) {
        port = config->Get<float>("Networking.Port", 27666);
    }
    m_Port = port;
    LOG_INFO("Server initialized and bound to port %i", port);
}

Server::~Server()
{

}

void Server::Update(double dt)
{
    m_EventBroker->Process<Server>();
    m_TimeStamp += dt;
    publishInputCommands();
    PlayerDefinition pd;
    m_Reliable.AcceptNewConnections(m_NextPlayerID, m_ConnectedPlayers);
    for (auto& kv : m_ConnectedPlayers) {
        while (kv.second.TCPSocket->available()) {
            // Packet will get real data in receive
            Packet packet(MessageType::Invalid);
            m_Reliable.Receive(packet, kv.second);
            m_Address = kv.second.TCPSocket->remote_endpoint().address();
            m_Port = kv.second.TCPSocket->remote_endpoint().port();
            if (packet.GetMessageType() == MessageType::Connect) {
                parseTCPConnect(packet);
            } else {
                parseMessageType(packet);
            }
        }
    }

    while (m_Unreliable.IsSocketAvailable()) {
        // Packet will get real data in receive
        Packet packet(MessageType::Invalid);
        m_Unreliable.Receive(packet, pd);
        m_Address = pd.Endpoint.address();
        m_Port = pd.Endpoint.port();
        if (packet.GetMessageType() == MessageType::Connect) {
            parseUDPConnect(packet);
        } else {
            parseMessageType(packet);
        }
    }
    // Check if players have disconnected
    for (int i = 0; i < m_PlayersToDisconnect.size(); i++) {
        disconnect(m_PlayersToDisconnect.at(i));
    }
    m_PlayersToDisconnect.clear();

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
    if (isReadingData) {
        Network::Update(dt);
    }
}

void Server::parseMessageType(Packet& packet)
{
    // Pop packetSize which is used by TCP Client to
    // create a packet of the correct size
    packet.ReadPrimitive<int>();

    int messageType = packet.ReadPrimitive<int>(); // Read what type off message was sent from server
    // Read packet ID 
    m_PreviousPacketID = m_PacketID;    // Set previous packet id
    m_PacketID = packet.ReadPrimitive<int>(); //Read new packet id
    //identifyPacketLoss();
    switch (static_cast<MessageType>(messageType)) {
    case MessageType::Connect:
        //parseConnect(packet);
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

void Server::reliableBroadcast(Packet& packet)
{
    for (auto& kv : m_ConnectedPlayers) {
        packet.ChangePacketID(kv.second.PacketID);
        m_Reliable.Send(packet, kv.second);
    }
}

void Server::unreliableBroadcast(Packet& packet)
{
    for (auto& kv : m_ConnectedPlayers) {
        packet.ChangePacketID(kv.second.PacketID);
        m_Unreliable.Send(packet, kv.second);
    }
}

// Send snapshot fields
void Server::sendSnapshot()
{
    Packet packet(MessageType::Snapshot);
    //addInputCommandsToPacket(packet);
    packet.WritePrimitive(m_TimeStamp/*+ somePingvalue + offset*/);
    addChildrenToPacket(packet, EntityID_Invalid);
    unreliableBroadcast(packet);
}

void Server::addInputCommandsToPacket(Packet& packet)
{
    // Number of input commands
    packet.WritePrimitive(m_InputCommandsToBroadcast.size());
    for (auto& command : m_InputCommandsToBroadcast) {
        packet.WritePrimitive(command.PlayerID);
        packet.WritePrimitive(m_ConnectedPlayers.at(command.PlayerID).EntityID);
        packet.WriteString(command.Command);
        packet.WritePrimitive(command.Value);
        packet.WritePrimitive(command.TimeStamp);
    }
    m_InputCommandsToBroadcast.clear();
}

void Server::addChildrenToPacket(Packet & packet, EntityID entityID)
{
    auto itPair = m_World->GetChildren(entityID);
    std::unordered_map<std::string, ComponentPool*> worldComponentPools = m_World->GetComponentPools();
    // Loop through every child
    for (auto it = itPair.first; it != itPair.second; it++) {
        EntityID childEntityID = it->second;
        // HACK: Only sync players for now, since the map turned out to be TOO LARGE to send in one snapshot and Simon's computer shits itself
        EntityWrapper childEntity(m_World, childEntityID);
        if (!shouldSendToClient(childEntity)) {
            continue;
        }

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
    reliableBroadcast(packet);
}

void Server::checkForTimeOuts()
{
    double startPing = 1000 * m_StartPingTime
        / static_cast<double>(CLOCKS_PER_SEC);

    std::vector<PlayerID> playersToRemove;
    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.TCPAddress != boost::asio::ip::address()) {
            int stopPing = 1000 * kv.second.StopTime /
                static_cast<double>(CLOCKS_PER_SEC);
            if (startPing > stopPing + m_TimeoutMs) {
                LOG_INFO("User %i timed out!", kv.second.Name);
                playersToRemove.push_back(kv.first);
            }
        }
    }
    for (size_t i = 0; i < playersToRemove.size(); i++) {
        disconnect(playersToRemove.at(i));
    }
}

void Server::parseUDPConnect(Packet & packet)
{
    // Pop size of message int
    packet.ReadPrimitive<int>();
    int messageType = packet.ReadPrimitive<int>();
    // Read packet ID 
    m_PreviousPacketID = m_PacketID;    // Set previous packet id
    m_PacketID = packet.ReadPrimitive<int>(); //Read new packet id
    // parse player id and other stuff
    PlayerID playerID = packet.ReadPrimitive<int>();
    // Do something here?
    boost::asio::ip::udp::endpoint endpoint(m_Address, m_Port);
    m_ConnectedPlayers.at(playerID).Endpoint = endpoint;
    LOG_INFO("parseUDPConnect: Spectator \"%s\" connected on IP: %s", m_ConnectedPlayers.at(playerID).Name.c_str(), m_ConnectedPlayers.at(playerID).Endpoint.address().to_string().c_str());
    // Send a message to the player that connected
    Packet connnectPacket(MessageType::Connect, m_ConnectedPlayers.at(playerID).PacketID);
    m_Unreliable.Send(connnectPacket);
    LOG_INFO("UDP Connect sent to client");
}

void Server::parseTCPConnect(Packet & packet)
{
    // Pop size of message int
    packet.ReadPrimitive<int>();
    int messageType = packet.ReadPrimitive<int>();
    // Read packet ID 
    m_PreviousPacketID = m_PacketID;    // Set previous packet id
    m_PacketID = packet.ReadPrimitive<int>(); //Read new packet id

    LOG_INFO("Parsing connections");
    // Check if player is already connected
    // Ska vara till lagd i TCPServer receive
    PlayerID playerID = GetPlayerIDFromEndpoint();
    if (playerID == -1) {
        return;
    }
    // Create a new player
    m_ConnectedPlayers.at(playerID).EntityID = 0; // Overlook this
    m_ConnectedPlayers.at(playerID).Name = packet.ReadString();
    m_ConnectedPlayers.at(playerID).PacketID = 0;
    m_ConnectedPlayers.at(playerID).StopTime = std::clock();
    m_ConnectedPlayers.at(playerID).TCPAddress = m_Address;
    m_ConnectedPlayers.at(playerID).TCPPort = m_Port;

    LOG_INFO("parseTCPConnect: Spectator \"%s\" connected on IP: %s", m_ConnectedPlayers.at(playerID).Name.c_str(),
        m_ConnectedPlayers.at(playerID).TCPAddress.to_string().c_str());

    // Send a message to the player that connected
    Packet connnectPacket(MessageType::Connect, m_ConnectedPlayers.at(playerID).PacketID);
    // Write playerID to packet
    connnectPacket.WritePrimitive(playerID);
    m_Reliable.Send(connnectPacket);

    // Send notification that a player has connected
    //Packet notificationPacket(MessageType::PlayerConnected);
    //broadcast(notificationPacket);
}

void Server::parseDisconnect()
{
    LOG_INFO("%i: Parsing disconnect", m_PacketID);

    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.TCPAddress == m_Address &&
            kv.second.TCPPort == m_Port) {
            m_PlayersToDisconnect.push_back(kv.first);
            break;
        }
    }
}

void Server::disconnect(PlayerID playerID)
{
    //broadcast("A player disconnected");
    LOG_INFO("User %s disconnected/timed out", m_ConnectedPlayers[playerID].Name.c_str());
    // Remove enteties and stuff (When we can remove entity, remove it and tell clients to remove the copy they have)
    Events::PlayerDisconnected e;
    e.Entity = m_ConnectedPlayers.at(playerID).EntityID;
    e.PlayerID = playerID;
    m_EventBroker->Publish(e);
    //m_World->DeleteEntity(m_ConnectedPlayers[playerID].EntityID);
    // TODO Kolla Anders crashade efter timeout med break point
    m_ConnectedPlayers[playerID].TCPSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    m_ConnectedPlayers[playerID].TCPSocket->close();
    m_World->DeleteEntity(m_ConnectedPlayers[playerID].EntityID);
    m_ConnectedPlayers.erase(playerID);
    // Send disconnect to the other players.
}

void Server::parseOnPlayerDamage(Packet & packet)
{
    Events::PlayerDamage e;
    e.Inflictor = EntityWrapper(m_World, packet.ReadPrimitive<EntityID>());
    e.Victim = EntityWrapper(m_World, packet.ReadPrimitive<EntityID>());
    e.Damage = packet.ReadPrimitive<double>();
    m_EventBroker->Publish(e);
    //LOG_DEBUG("Server::parseOnPlayerDamage: Command is %s. Value is %f. PlayerID is %i.", e.DamageAmount, e.PlayerDamagedID, e.TypeOfDamage.c_str());
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
    m_Reliable.Send(packet);
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
    } else if (e.Command == "KickPlayer" && e.Value > 0) {
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
    m_Reliable.Send(packet, m_ConnectedPlayers[e.PlayerID]);
    return false;
}

bool Server::OnEntityDeleted(const Events::EntityDeleted & e)
{
    if (!e.Cascaded) {
        Packet packet = Packet(MessageType::EntityDeleted);
        packet.WritePrimitive<EntityID>(e.DeletedEntity);
        reliableBroadcast(packet);
    }
    return false;
}

bool Server::OnComponentDeleted(const Events::ComponentDeleted & e)
{
    if (!e.Cascaded) {
        if (shouldSendToClient(EntityWrapper(m_World, e.Entity))) {
            Packet packet = Packet(MessageType::ComponentDeleted);
            packet.WritePrimitive<EntityID>(e.Entity);
            packet.WriteString(e.ComponentType);
            reliableBroadcast(packet);
        }
    }
    return false;
}

bool Server::OnPlayerDamage(const Events::PlayerDamage& e)
{
    Packet packet(MessageType::OnPlayerDamage);
    packet.WritePrimitive(e.Inflictor.ID);
    packet.WritePrimitive(e.Victim.ID);
    packet.WritePrimitive(e.Damage);
    reliableBroadcast(packet);

    return false;
}

void Server::parseClientPing()
{
    LOG_INFO("%i: Parsing ping", m_PacketID);
    PlayerID player = GetPlayerIDFromEndpoint();
    if (player == -1) {
        return;
    }
    // Return ping
    Packet packet(MessageType::Ping, m_ConnectedPlayers[player].PacketID);
    packet.WriteString("Ping received");
    m_Reliable.Send(packet);
}

void Server::parsePing()
{
    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.TCPAddress == m_Address &&
            kv.second.TCPPort == m_Port) {
            kv.second.StopTime = std::clock();
            break;
        }
    }
}

void Server::parseOnInputCommand(Packet& packet)
{
    PlayerID player = -1;
    // Check which player it was who sent the message
    player = GetPlayerIDFromEndpoint();
    if (player != -1) {
        while (packet.DataReadSize() < packet.Size()) {
            Events::InputCommand e;
            e.Command = packet.ReadString();
            e.PlayerID = player; // Set correct player id
            e.Player = EntityWrapper(m_World, m_ConnectedPlayers.at(player).EntityID);
            e.Value = packet.ReadPrimitive<float>();
            e.TimeStamp = packet.ReadPrimitive<double>();
            /* m_EventBroker->Publish(e);*/
            m_InputCommandsToPublish.push_back(e);
            if (e.Command == "PrimaryFire" || e.Command == "Reload") {
                m_InputCommandsToBroadcast.push_back(e);
            }
            //LOG_INFO("Server::parseOnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
        }
    }
}

void Server::parsePlayerTransform(Packet& packet)
{
    PlayerID playerID = GetPlayerIDFromEndpoint();
    if (playerID == -1) {
        return;
    }

    glm::vec3 position;
    glm::vec3 orientation;
    position.x = packet.ReadPrimitive<float>();
    position.y = packet.ReadPrimitive<float>();
    position.z = packet.ReadPrimitive<float>();
    orientation.x = packet.ReadPrimitive<float>();
    orientation.y = packet.ReadPrimitive<float>();
    orientation.z = packet.ReadPrimitive<float>();

    bool hasAssaultWeapon = packet.ReadPrimitive<bool>();
    int magazineAmmo;
    int ammo;
    if (hasAssaultWeapon) {
        magazineAmmo = packet.ReadPrimitive<int>();
        ammo = packet.ReadPrimitive<int>();
    }

    EntityWrapper player(m_World, m_ConnectedPlayers.at(playerID).EntityID);
    if (player.Valid()) {
        player["Transform"]["Position"] = position;
        player["Transform"]["Orientation"] = orientation;

        if (hasAssaultWeapon) {
            player["AssaultWeapon"]["MagazineAmmo"] = magazineAmmo;
            player["AssaultWeapon"]["Ammo"] = ammo;
        }
    }
}

bool Server::shouldSendToClient(EntityWrapper childEntity)
{
    return childEntity.HasComponent("Player") || childEntity.FirstParentWithComponent("Player").Valid();
}

void Server::publishInputCommands()
{
    std::vector<Events::InputCommand> notPublishedEvents;
    for (int i = 0; i < m_InputCommandsToPublish.size(); i++) {
        if (m_InputCommandsToPublish.at(i).TimeStamp < m_TimeStamp) {
            m_EventBroker->Publish(m_InputCommandsToPublish.at(i));
        } else {
            LOG_INFO("Did not instantly publish command");
            notPublishedEvents.push_back(m_InputCommandsToPublish.at(i));
        }
    }
    m_InputCommandsToPublish = notPublishedEvents;
}

PlayerID Server::GetPlayerIDFromEndpoint()
{
    // check both tcp and udp connection
    for (auto& kv : m_ConnectedPlayers) {
        if ((kv.second.TCPAddress == m_Address
            && kv.second.TCPPort == m_Port)
            || (kv.second.Endpoint.address() == m_Address
                && kv.second.Endpoint.port() == m_Port)) {
            return kv.first;
        }
    }
    return -1;
}