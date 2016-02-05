#include "Network/Server.h"

Server::Server()
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
        parseConnect(packet);
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

void Server::broadcast(Packet& packet)
{
    for (auto& kv : m_ConnectedPlayers) {
        packet.ChangePacketID(kv.second.PacketID);
        send(packet, kv.second);
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

    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.Address != boost::asio::ip::address()) {
            int stopPing = 1000 * kv.second.StopTime /
                static_cast<double>(CLOCKS_PER_SEC);
            if (startPing > stopPing + m_TimeoutMs) {
                LOG_INFO("User %i timed out!", kv.second.Name);
                disconnect(kv.first);
            }
        }
    }
}

void Server::parseDisconnect()
{
    LOG_INFO("%i: Parsing disconnect", m_PacketID);

    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.Address == m_Address &&
            kv.second.Port == m_Port) {
            disconnect(kv.first);
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

    m_ConnectedPlayers.erase(playerID);
}

void Server::parseOnPlayerDamage(Packet & packet)
{
    Events::PlayerDamage e;
    e.Damage = packet.ReadPrimitive<double>();
    e.Player = EntityWrapper(m_World, packet.ReadPrimitive<EntityID>());
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
    send(packet);
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
    send(packet, m_ConnectedPlayers[e.PlayerID]);
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
    send(packet);
}

void Server::parsePing()
{
    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.Address == m_Address &&
            kv.second.Port == m_Port) {
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
            m_EventBroker->Publish(e);
            LOG_INFO("Server::parseOnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
        }
    }
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

    PlayerID playerID = GetPlayerIDFromEndpoint();
    EntityWrapper player(m_World, m_ConnectedPlayers.at(playerID).EntityID);

    if (player.Valid()) {
        player["Transform"]["Position"] = position;
        player["Transform"]["Orientation"] = orientation;
    }
}

PlayerID Server::GetPlayerIDFromEndpoint()
{
    for (auto& kv : m_ConnectedPlayers) {
        if (kv.second.Address == m_Address &&
            kv.second.Port == m_Port) {
            return kv.first;
        }
    }
    return -1;
}