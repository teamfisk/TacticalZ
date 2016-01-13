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
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        m_StopTimes[i] = std::clock();
    }
    LOG_INFO("I am Server. BIP BOP\n");
}

void Server::Update()
{
    readFromClients();
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
        //checkForTimeOuts();
        timOutTimer = currentTime;
    }
}

void Server::parseMessageType(Packet& packet)
{
    int messageType = packet.ReadPrimitive<int>(); // Read what type off message was sent from server

    // Read packet ID 
    m_PreviousPacketID = m_PacketID;    // Set previous packet id
    m_PacketID = packet.ReadPrimitive<int>(); //Read new packet id
    //IdentifyPacketLoss();
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
    case MessageType::Event:
        break;
    case MessageType::OnInputCommand:
        parseOnInputCommand(packet);
        break;;
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

void Server::moveMessageHead(char *& data, size_t & length, size_t stepSize)
{
    data += stepSize;
    length -= stepSize;
}

void Server::broadcast(std::string message)
{
    Packet packet(MessageType::Event, m_SendPacketID);
    packet.WriteString(message);
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            send(packet, i);
        }
    }
}

void Server::broadcast(Packet& packet)
{
    for (int i = 0; i < MAXCONNECTIONS; ++i) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
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
        Packet packet(MessageType::Snapshot, m_SendPacketID);
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
            int ping = 1000 * (m_StopTimes[i] - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
            LOG_INFO("Last packetID received %i: Player %i's ping: %i", m_PacketID, i, ping);
        }
    }
    // Create ping message
    Packet packet(MessageType::ServerPing, m_SendPacketID);
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
            int stopPing = 1000 * m_StopTimes[i]
                / static_cast<double>(CLOCKS_PER_SEC);
            if (startPing > stopPing + timeOutTimeMs) {
                LOG_INFO("Player %i timed out!", i);
                disconnect(i);
            }
        }
    }
}

void Server::disconnect(int i)
{
    broadcast("A player disconnected");
    LOG_INFO("Player %i disconnected/timed out", i);

    // Remove enteties and stuff
    m_PlayerDefinitions[i].Endpoint = boost::asio::ip::udp::endpoint();
    m_PlayerDefinitions[i].EntityID = -1;
    m_PlayerDefinitions[i].Name = "";
}

void Server::parseOnInputCommand(Packet& packet)
{
    size_t i;
    for (i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            break;
        }
    }
    // If no player matches the address return.
    if (i >= 8)
        return;
    Events::InputCommand e;
    e.Command = packet.ReadString();
    e.PlayerID = packet.ReadPrimitive<int>();
    e.Value = packet.ReadPrimitive<float>();
    m_EventBroker->Publish(e);
    LOG_INFO("Client::OnInputCommand: Command is %s. Value is %f. PlayerID is %i.", e.Command.c_str(), e.Value, e.PlayerID);
}

void Server::parseConnect(Packet& packet)
{
    LOG_INFO("Parsing connections");
    // Check if player is already connected
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            return;
        }
    }

    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == boost::asio::ip::address()) {
            // Create new player
            m_PlayerDefinitions[i].EntityID = createPlayer();
            m_PlayerDefinitions[i].Endpoint = m_ReceiverEndpoint;
            m_PlayerDefinitions[i].Name = packet.ReadString();

            m_StopTimes[i] = std::clock();

            LOG_INFO("Player \"%s\" connected on IP: %s", m_PlayerDefinitions[i].Name.c_str(), m_PlayerDefinitions[i].Endpoint.address().to_string().c_str());

            Packet packet(MessageType::Connect, m_SendPacketID);
            packet.WritePrimitive<int>(i); // Player ID

            send(packet, i);

            // Send notification that a player has connected
            std::string str = m_PacketID + "Player " + m_PlayerDefinitions[i].Name + " connected on: "
                + m_PlayerDefinitions[i].Endpoint.address().to_string();
            broadcast(str);
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
    // Return ping
    Packet packet(MessageType::ClientPing, m_SendPacketID);
    packet.WriteString("Ping received");
    send(packet); // This dosen't work for multiple users
}

void Server::parseServerPing()
{
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            m_StopTimes[i] = std::clock();
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
