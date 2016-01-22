#include "Network/Client.h"

using namespace boost::asio::ip;


Client::Client(ConfigFile* config) : m_Socket(m_IOService)
{
    Network::initialize();

    // Asumes root node is EntityID 0
    insertIntoServerClientMaps(0, 0);
    // Init timer
    m_TimeSinceSentInputs = std::clock();
    // Default is local host
    std::string address = config->Get<std::string>("Networking.Address", "127.0.0.1");
    int port = config->Get<int>("Networking.Port", 13);
    m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address::from_string(address), port);
    // Set up network stream
    m_PlayerName = config->Get<std::string>("Networking.Name", "Raptorcopter");
    m_SendInputIntervalMs = config->Get<int>("Networking.SendInputIntervalMs", 33);

}

Client::~Client()
{ }

void Client::Start(World* world, EventBroker* eventBroker)
{
    m_EventBroker = eventBroker;
    m_World = world;

    // Subscribe to events
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &Client::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayeDamage, &Client::OnPlayerDamage);

    m_Socket.connect(m_ReceiverEndpoint);
    LOG_INFO("I am client. BIP BOP");
}

void Client::Update()
{
    m_EventBroker->Process<Client>();
    readFromServer();
    if (m_IsConnected) {
        hasServerTimedOut();
        // Don't sent 1 input in 1 packet, bunch em up.
        if (m_SendInputIntervalMs < (1000 * (std::clock() - m_TimeSinceSentInputs) / (double)CLOCKS_PER_SEC)) {
            sendInputCommands();
            m_TimeSinceSentInputs = std::clock();
        }
    }
    Network::Update();
}

void Client::readFromServer()
{
    while (m_Socket.available()) {
        bytesRead = receive(readBuf);
        if (bytesRead > 0) {
            Packet packet(readBuf, bytesRead);
            parseMessageType(packet);
        }
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
    identifyPacketLoss();

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
        parseSnapshot(packet);
        break;
    case MessageType::Disconnect:
        break;
    case MessageType::PlayerConnected:
        parsePlayerConnected(packet);
    case MessageType::Kick:
        parseKick();
        break;
    default:
        break;
    }
}

void Client::parseConnect(Packet& packet)
{
    // Map ServerEntityID and your PlayerID
    LOG_INFO("I be connected PogChamp");
}

void Client::parsePlayerConnected(Packet & packet)
{
    // Map ServerEntityID and other player's PlayerID
    LOG_INFO("A Player connected");
}

void Client::parsePing()
{
    // Might miss connect message so set it here instead.
    m_IsConnected = true;
    // Time since last ping was received
    m_DurationOfPingTime = 1000 * (std::clock() - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
    LOG_INFO("%i: response time with ctime(ms): %f", m_PacketID, m_DurationOfPingTime);
    m_StartPingTime = std::clock();

    Packet packet(MessageType::Ping, m_SendPacketID);
    packet.WriteString("Ping recieved");
    send(packet);
}

void Client::parseKick()
{ 
    LOG_WARNING("You have been kicked from the server.");
    m_IsConnected = false;
}

// Fields with strings will not work right now
void Client::InterpolateFields(Packet& packet, const ComponentInfo& componentInfo, const EntityID& entityID, const std::string& componentType)
{
    int sizeOfFields = 0;
    for (auto field : componentInfo.FieldsInOrder) {
        ComponentInfo::Field_t fieldInfo = componentInfo.Fields.at(field);
        sizeOfFields += fieldInfo.Stride;
    }
    // Is the size correct?
    boost::shared_array<char> eventData(new char[componentInfo.Stride]);
    memcpy(eventData.get(), packet.ReadData(componentInfo.Stride), componentInfo.Stride);
    //Send event to interpolat system
    Events::Interpolate e;
    e.Entity = entityID;
    e.DataArray = eventData;
    m_EventBroker->Publish(e);

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
        if (serverClientMapsHasEntity(receivedEntityID)) {
            // Get the local EntityID
            EntityID entityID = m_ServerIDToClientID.at(receivedEntityID);
            // Check if the component exists
            if (m_World->HasComponent(entityID, componentType)) {
                // If the entity and the component exists update it
                if (componentType == "Transform") {
                    InterpolateFields(packet, componentInfo, entityID, componentType);
                } else {
                    updateFields(packet, componentInfo, entityID, componentType);
                }
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
            insertIntoServerClientMaps(receivedEntityID, newEntityID);
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
            if (serverClientMapsHasEntity(receivedParentEntityID)) {
                m_World->SetParent(m_ServerIDToClientID.at(receivedEntityID), m_ServerIDToClientID.at(receivedParentEntityID));
                // If Parent dosen't exist create one and map receivedParentEntityID to it.
            } else {
                // Create the new parent and add it to map
                EntityID newParentEntityID = m_World->CreateEntity();
                insertIntoServerClientMaps(receivedParentEntityID, newParentEntityID);
                // Set the newly created Entity as parent.
                m_World->SetParent(m_ServerIDToClientID.at(receivedEntityID), newParentEntityID);
            }
        }
    }
}

int Client::receive(char* data)
{
    boost::system::error_code error;

    int bytesReceived = m_Socket.receive_from(boost
        ::asio::buffer((void*)data, INPUTSIZE),
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

void Client::send(Packet& packet)
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

void Client::connect()
{
    Packet packet(MessageType::Connect, m_SendPacketID);
    packet.WriteString(m_PlayerName);
    m_StartPingTime = std::clock();
    send(packet);
}

void Client::disconnect()
{
    m_PreviousPacketID = 0;
    m_PacketID = 0;
    Packet packet(MessageType::Disconnect, m_SendPacketID);
    send(packet);
}

bool Client::OnInputCommand(const Events::InputCommand & e)
{
    if (e.Command == "ConnectToServer") { // Connect for now
        if (e.Value > 0) {
            connect();
        }
        //LOG_DEBUG("Client::OnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
        return true;
    } else if (e.Command == "DisconnectFromServer") {
        if (e.Value > 0) {
            disconnect();
        }
        return true;
    } else if (e.Command == "SwitchToPlayer") {
        if (e.Value > 0) {
            becomePlayer();
        }
    } else if (e.Command == "LogNetworkBandwidth") {
        if (e.Value > 0) {
            // Save to file if we no longer want to read data.
            if (isReadingData) {
                saveToFile();
            }
            isReadingData = !isReadingData;
            m_SaveDataTimer = std::clock();
        }
    } else {
        m_InputCommandBuffer.push_back(e);
        //LOG_DEBUG("Client::OnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
        return true;
    }
    return false;
}

bool Client::OnPlayerDamage(const Events::PlayerDamage & e)
{
    Packet packet(MessageType::OnInputCommand, m_SendPacketID);
    packet.WritePrimitive(e.DamageAmount);
    packet.WritePrimitive(e.PlayerDamagedID);
    send(packet);
    return false;
}

void Client::identifyPacketLoss()
{
    // if no packets lost, difference should be equal to 1
    int difference = m_PacketID - m_PreviousPacketID;
    if (difference != 1) {
        LOG_INFO("%i Packet(s) were lost...", difference - 1);
    }
}

bool Client::hasServerTimedOut()
{
    // Time in ms
    float timeSincePing = 1000 * (std::clock() - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
    if (timeSincePing > m_TimeoutMs) {
        // Clear everything and go to menu.
        LOG_INFO("Server has timed out, returning to menu, Beep Boop.");
        m_IsConnected = false;
        return true;
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

void Client::sendInputCommands()
{
    if (m_InputCommandBuffer.size() > 0) {
        Packet packet(MessageType::OnInputCommand, m_SendPacketID);
        for (int i = 0; i < m_InputCommandBuffer.size(); i++) {
            packet.WriteString(m_InputCommandBuffer[i].Command);
            packet.WritePrimitive(m_InputCommandBuffer[i].Value);
        }
        send(packet);
        m_InputCommandBuffer.clear();
    }
}

void Client::becomePlayer()
{
    Packet packet = Packet(MessageType::BecomePlayer, m_SendPacketID);
    send(packet);
}

bool Client::clientServerMapsHasEntity(EntityID clientEntityID)
{
    return m_ClientIDToServerID.find(clientEntityID) != m_ClientIDToServerID.end();
}

bool Client::serverClientMapsHasEntity(EntityID serverEntityID)
{
    return m_ServerIDToClientID.find(serverEntityID) != m_ServerIDToClientID.end();
}

void Client::insertIntoServerClientMaps(EntityID serverEntityID, EntityID clientEntityID)
{
    m_ServerIDToClientID.insert(std::make_pair(serverEntityID, clientEntityID));
    m_ClientIDToServerID.insert(std::make_pair(clientEntityID, serverEntityID));

}
