#include "Network/Client.h"

using namespace boost::asio::ip;


Client::Client(ConfigFile* config) : m_Socket(m_IOService)
{
    // Asumes root node is EntityID 0
    m_ServerToClientMap.insert(std::make_pair(0, 0));
    // Default is local host
    std::string address = config->Get<std::string>("Networking.Address", "127.0.0.1");
    int port = config->Get<int>("Networking.Port", 13);
    m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address::from_string(address), port);
    // Set up network stream
    m_PlayerName = config->Get<std::string>("Networking.Name", "Raptorcopter");
    m_NextSnapshot.InputForward = "";
    m_NextSnapshot.InputRight = "";
}

Client::~Client()
{ }

void Client::Start(World* world, EventBroker* eventBroker)
{
    m_WasStarted = true;
    m_EventBroker = eventBroker;
    m_World = world;

    // Subscribe to events
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &Client::OnInputCommand);

    m_Socket.connect(m_ReceiverEndpoint);
    LOG_INFO("I am client. BIP BOP");
}

void Client::Update()
{
    readFromServer();
}

void Client::readFromServer()
{
    while (m_Socket.available()) {
        bytesRead = receive(readBuf, INPUTSIZE);
        if (bytesRead > 0) {
            Packet packet(readBuf, bytesRead);
            parseMessageType(packet);
        }
    }
    std::clock_t currentTime = std::clock();
    if (snapshotInterval < (1000 * (currentTime - previousSnapshotMessage) / (double)CLOCKS_PER_SEC)) {
        if (isConnected()) {
            //sendSnapshotToServer();
        }
        previousSnapshotMessage = currentTime;
    }
}

void Client::sendInputEvents()
{
    // Reset previous key state in snapshot.
    m_NextSnapshot.InputForward = "";
    m_NextSnapshot.InputRight = "";

    auto player = m_World->GetComponent(m_PlayerDefinitions[m_PlayerID].EntityID, "Player");

    // See if any movement keys are down
    // We dont care if it's overwritten by later
    // if statement. Watcha gonna do, right!
    if (player["Forward"]) {
        m_NextSnapshot.InputForward = "+Forward";
    }
    if (player["Left"]) {
        m_NextSnapshot.InputRight = "-Right";
    }
    if (player["Back"]) {
        m_NextSnapshot.InputForward = "-Forward";
    }
    if (player["Right"]) {
        m_NextSnapshot.InputRight = "+Right";
    }

    if (m_NextSnapshot.InputForward != "") {
        Packet packet(MessageType::Event, m_SendPacketID);
        packet.WriteString(m_NextSnapshot.InputForward);
        send(packet);
    } else {
        Packet packet(MessageType::Event, m_SendPacketID);
        packet.WriteString("0Forward");
        send(packet);
    }

    if (m_NextSnapshot.InputRight != "") {
        Packet packet(MessageType::Event, m_SendPacketID);
        packet.WriteString(m_NextSnapshot.InputRight);
        send(packet);
    } else {
        Packet packet(MessageType::Event, m_SendPacketID);
        packet.WriteString("0Right");
        send(packet);
    }
}

void Client::parseMessageType(Packet& packet)
{
    int messageType = packet.ReadPrimitive<int>();
    if (messageType == -1)
        return;
    // Read packet ID 
    m_PreviousPacketID = m_PacketID;    // Set previous packet id
    m_PacketID = packet.ReadPrimitive<int>(); //Read new packet id
    if (m_PacketID <= m_PreviousPacketID)
        return;
    //IdentifyPacketLoss();

    switch (static_cast<MessageType>(messageType)) {
    case MessageType::Connect:
        parseConnect(packet);
        break;
    case MessageType::ClientPing:
        parsePing();
        break;
    case MessageType::ServerPing:
        parseServerPing();
        break;
    case MessageType::Message:
        break;
    case MessageType::Snapshot:
        parseSnapshot(packet);
        break;
    case MessageType::Disconnect:
        break;
    case MessageType::Event:
        parseEventMessage(packet);
        break;
    default:
        break;
    }
}

void Client::parseConnect(Packet& packet)
{
    m_PlayerID = packet.ReadPrimitive<int>();
    LOG_INFO("%i: I am player: %i", m_PacketID, m_PlayerID);
}

void Client::parsePing()
{
    m_DurationOfPingTime = 1000 * (std::clock() - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
    LOG_INFO("%i: response time with ctime(ms): %f", m_PacketID, m_DurationOfPingTime);
}

void Client::parseServerPing()
{
    Packet packet(MessageType::ServerPing, m_SendPacketID);
    packet.WriteString("Ping recieved");
    send(packet);
}

void Client::parseEventMessage(Packet& packet)
{
    int Id = -1;
    std::string command = packet.ReadString();
    if (command.find("+Player") != std::string::npos) {
        Id = packet.ReadPrimitive<int>();
        // Sett Player name
        m_PlayerDefinitions[Id].Name = command.erase(0, 7);
    } else {
        LOG_INFO("%i: Event message: %s", m_PacketID, command.c_str());
    }
}

void Client::updateFields(Packet& packet, const ComponentInfo& componentInfo, const EntityID& entityID, const std::string& componentType)
{
    for (auto field : componentInfo.FieldsInOrder) {
        ComponentInfo::Field_t fieldInfo = componentInfo.Fields.at(field);
        if (fieldInfo.Type == "string") {
            std::string& value = packet.ReadString();
            m_World->GetComponent(entityID, componentType)[fieldInfo.Name] = value;
        } else {
            memcpy(m_World->GetComponent(entityID, componentType).Data + fieldInfo.Offset, packet.ReadData(fieldInfo.Stride), fieldInfo.Stride);
        }
    }
}

// Field parse
void Client::parseSnapshot(Packet& packet)
{
    std::string componentType = packet.ReadString();
    while (packet.DataReadSize() < packet.Size()) {
        // Components EntityID
        EntityID receivedEntityID = packet.ReadPrimitive<EntityID>();
        // Parents EntityID 
        EntityID receivedParentEntityID = packet.ReadPrimitive<EntityID>();
        ComponentInfo componentInfo = m_World->GetComponents(componentType)->ComponentInfo();
        // Check if the received EntityID is mapped to one of our local EntityIDs
        if (hasMappedEntity(receivedEntityID)) {
            // Get the local EntityID
            EntityID entityID = m_ServerToClientMap.at(receivedEntityID);
            // Check if the component exists
            if (m_World->HasComponent(entityID, componentType)) {
                // If the entity and the component exists update it
                updateFields(packet, componentInfo, entityID, componentType);
                // if entity exists but not the component
            } else {
                // Create component
                m_World->AttachComponent(entityID, componentType);
                // Copy data to newly created component
                updateFields(packet, componentInfo, entityID, componentType);
            }
            // If the entity dosent exist nor the component
        } else {
            // Create Entity
            // If entity dosen't exist
            EntityID newEntityID = m_World->CreateEntity();
            m_ServerToClientMap.insert(std::make_pair(receivedEntityID, newEntityID));
            // Check if EntityIDs are out of sync
            if (newEntityID != receivedEntityID) {
                LOG_INFO("Client::parseSnapshot(Packet& packet): Newly created EntityID is not the \
                            same as the one sent by server (EntityIDs are out of sync)");
            }
            // Create component
            m_World->AttachComponent(newEntityID, componentType);
            // Copy data to newly created component
            updateFields(packet, componentInfo, newEntityID, componentType);
        }

        // Parent Logic
        // Don't need to check if receivedEntityID is mapped. (It should have been set) 
        if (receivedParentEntityID != std::numeric_limits<EntityID>::max()) {
            if (hasMappedEntity(receivedParentEntityID)) {
                m_World->SetParent(m_ServerToClientMap.at(receivedEntityID), m_ServerToClientMap.at(receivedParentEntityID));
                // If Parent dosen't exist create one and map receivedParentEntityID to it.
            } else {
                // Create the new parent and add it to map
                EntityID newParentEntityID = m_World->CreateEntity();
                m_ServerToClientMap.insert(std::make_pair(receivedParentEntityID, newParentEntityID));
                // Set the newly created Entity as parent.
                m_World->SetParent(m_ServerToClientMap.at(receivedEntityID), newParentEntityID);
            }
        }
    }
}

int Client::receive(char* data, size_t length)
{
    boost::system::error_code error;

    int bytesReceived = m_Socket.receive_from(boost
        ::asio::buffer((void*)data, length),
        m_ReceiverEndpoint,
        0, error);

    if (error) {
        LOG_ERROR("receive: %s", error.message().c_str());
    }
    return bytesReceived;
}

void Client::send(Packet& packet)
{
    m_Socket.send_to(boost::asio::buffer(
        packet.Data(),
        packet.Size()),
        m_ReceiverEndpoint, 0);
}

void Client::connect()
{
    Packet packet(MessageType::Connect, m_SendPacketID);
    packet.WriteString(m_PlayerName);
    m_StartPingTime = std::clock();
    send(packet);
}

void Client::disconnect()
{
    Packet packet(MessageType::Connect, m_SendPacketID);
    packet.WriteString("+Disconnect");
    send(packet);
}

void Client::ping()
{
    Packet packet(MessageType::Connect, m_SendPacketID);
    packet.WriteString("Ping");
    m_StartPingTime = std::clock();
    send(packet);
}

void Client::moveMessageHead(char*& data, size_t& length, size_t stepSize)
{
    data += stepSize;
    length -= stepSize;
}

bool Client::OnInputCommand(const Events::InputCommand & e)
{
    if (e.Command == "ConnectToServer") { // Connect for now
        connect();
        LOG_DEBUG("Client::OnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
        return true;
    } else {
        Packet packet(MessageType::OnInputCommand, m_SendPacketID);
        packet.WriteString(e.Command);
        packet.WritePrimitive(e.PlayerID);
        packet.WritePrimitive(e.Value);
        send(packet);
        LOG_DEBUG("Client::OnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
        return true;
    }
    return false;
}

bool Client::OnPlayerDamage(const Events::PlayerDamage & e)
{
    Packet packet(MessageType::OnInputCommand, m_SendPacketID);
    packet.WritePrimitive(e.DamageAmount);
    packet.WritePrimitive(e.PlayerDamagedID);
    packet.WriteString(e.TypeOfDamage);
    send(packet);
    return false;
}

void Client::identifyPacketLoss()
{
    // if no packets lost, difference should be equal to 1
    int difference = m_PacketID - m_PreviousPacketID;
    if (difference != 1) {
        LOG_INFO("%i Packet(s) were lost...", difference);
    }
}

bool Client::isConnected()
{
    if (m_PlayerID != -1) {
        if (m_PlayerDefinitions[m_PlayerID].EntityID != -1) {
            return true;
        }
    }
    return false;
}

EntityID Client::createPlayer()
{
    EntityID entityID = m_World->CreateEntity();
    ComponentWrapper transform = m_World->AttachComponent(entityID, "Transform");
    ComponentWrapper model = m_World->AttachComponent(entityID, "Model");
    model["Resource"] = "Models/Core/UnitSphere.obj";
    ComponentWrapper player = m_World->AttachComponent(entityID, "Player");
    return entityID;
}

bool Client::hasMappedEntity(EntityID entityID)
{
    return m_ServerToClientMap.find(entityID) != m_ServerToClientMap.end();
}
