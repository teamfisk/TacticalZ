#include "Network/Server.h"

Server::Server() : m_Socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 13))
{ }

Server::~Server()
{ }


void Server::Start(World* world, EventBroker* eventBroker)
{
    m_World = world;
    m_EventBroker = eventBroker;
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        m_StopTimes[i] = std::clock();
    }
    LOG_INFO("I am Server. BIP BOP\n");

    ReadFromClients();
}

void Server::Update()
{
    while (m_PlayersToCreate.size() > 0) {
        unsigned int i = m_PlayersToCreate.size() - 1;
        unsigned int tempID = m_World->CreateEntity();
        ComponentWrapper transform = m_World->AttachComponent(tempID, "Transform");
        transform["Position"] = glm::vec3(-1.5f, 0.f, 0.f);
        ComponentWrapper model = m_World->AttachComponent(tempID, "Model");
        model["Resource"] = "Models/Core/UnitSphere.obj";
        model["Color"] = glm::vec4(rand()%255 / 255.f, rand()%255 / 255.f, rand() %255 / 255.f, 1.f);
        ComponentWrapper player = m_World->AttachComponent(tempID, "Player");
        m_PlayerDefinitions[m_PlayersToCreate[i]].EntityID = tempID;
        m_PlayersToCreate.pop_back();
    }
}

void Server::Close()
{
    m_ThreadIsRunning = false;
}

void Server::ReadFromClients()
{
    char readBuffer[1024] = { 0 };
    int bytesRead = 0;
    // time for previouse message
    std::clock_t previousePingMessage = std::clock();
    std::clock_t previousSnapshotMessage = std::clock();
    std::clock_t timOutTimer = std::clock();
    // How often we send messages (milliseconds)
    int intervalMs = 1000;
    int snapshotInterval = 50;
    int checkTimeOutInterval = 100;

    while (m_ThreadIsRunning) {
        // m_ThreadIsRunning might be unnecessary but the 
        // program crashed if it executed m_Socket.available()
        // when closing the program. 

        if (m_ThreadIsRunning && m_Socket.available()) {
            try {
                bytesRead = Receive(readBuffer, INPUTSIZE);
                Package package(readBuffer, bytesRead);
                ParseMessageType(package);
            } catch (const std::exception& err) {
                LOG_ERROR("%i: Read from client crashed %s", m_PacketID, err.what());
            }

        }
        std::clock_t currentTime = std::clock();
        // Send snapshot
        if (snapshotInterval < (1000 * (currentTime - previousSnapshotMessage) / (double)CLOCKS_PER_SEC)) {
            SendSnapshot();
            previousSnapshotMessage = currentTime;
        }

        // Send pings each 
        if (intervalMs < (1000 * (currentTime - previousePingMessage) / (double)CLOCKS_PER_SEC)) {
            SendPing();
            previousePingMessage = currentTime;
        }

        // Time out logic
        if (checkTimeOutInterval < (1000 * (currentTime - timOutTimer) / (double)CLOCKS_PER_SEC)) {
            CheckForTimeOuts();
            timOutTimer = currentTime;
        }
    }
}

void Server::ParseMessageType(Package& package)
{
    int messageType = package.PopFrontPrimitive<int>(); // Read what type off message was sent from server

    // Read packet ID 
    m_PreviousPacketID = m_PacketID;    // Set previous packet id
    m_PacketID = package.PopFrontPrimitive<int>(); //Read new packet id
    IdentifyPacketLoss();
    switch (static_cast<MessageType>(messageType)) {
    case MessageType::Connect:
        ParseConnect(package);
        break;
    case MessageType::ClientPing:
        //ParseClientPing();
        break;
    case MessageType::ServerPing:
        ParseServerPing();
        break;
    case MessageType::Message:
        break;
    case MessageType::Snapshot:
        ParseSnapshot(package);
        break;
    case MessageType::Disconnect:
        ParseDisconnect();
        break;
    case MessageType::Event:
        ParseEvent(package);
        break;
    default:
        break;
    }
}

int Server::Receive(char * data, size_t length)
{
    length = m_Socket.receive_from(
        boost::asio::buffer((void*)data
            , length)
        , m_ReceiverEndpoint, 0);
    return length;
}

void Server::Send(Package& message, int playerID)
{
    m_Socket.send_to(
        boost::asio::buffer(message.Data(), message.Size()),
        m_PlayerDefinitions[playerID].Endpoint,
        0);
}

void Server::Send(Package & package)
{
    m_Socket.send_to(
        boost::asio::buffer(
            package.Data(),
            package.Size()),
        m_ReceiverEndpoint,
        0);
}

void Server::MoveMessageHead(char *& data, size_t & length, size_t stepSize)
{
    data += stepSize;
    length -= stepSize;
}

void Server::Broadcast(std::string message)
{
    Package package(MessageType::Event, m_SendPacketID);
    package.AddString(message);
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            Send(package, i);
        }
    }
}

void Server::Broadcast(Package& package)
{
    for (int i = 0; i < MAXCONNECTIONS; ++i) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            Send(package, i);
        }
    }
}

void Server::SendSnapshot()
{
    Package package(MessageType::Snapshot, m_SendPacketID);
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {

        // Send an empty name if there is no player connected on this position.
        package.AddString(m_PlayerDefinitions[i].Name);

        if (m_PlayerDefinitions[i].EntityID == -1) {
            continue;
        }
        // Pack transfrom component into data package
        auto transform = m_World->GetComponent(m_PlayerDefinitions[i].EntityID, "Transform");
        package.AddData(transform.Data, transform.Info.Meta.Stride);
    }
    Broadcast(package);
}

void Server::SendPing()
{
    // Prints connected players ping
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            int ping = 1000 * (m_StopTimes[i] - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
            LOG_INFO("%i: Player %i's ping: %i", m_PacketID, i, ping);
        }
    }

    // Create ping message
    Package package(MessageType::ServerPing, m_SendPacketID);
    package.AddString("Ping from server");
    // Time message
    m_StartPingTime = std::clock();
    // Send message
    Broadcast(package);
}

void Server::CheckForTimeOuts()
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
                Disconnect(i);
            }
        }
    }
}

void Server::Disconnect(int i)
{
    Broadcast("A player disconnected");
    LOG_INFO("Player %i disconnected/timed out", i);

    // Remove enteties and stuff
    m_PlayerDefinitions[i].Endpoint = boost::asio::ip::udp::endpoint();
    m_PlayerDefinitions[i].EntityID = -1;
    m_PlayerDefinitions[i].Name = "";
}

void Server::ParseEvent(Package& package)
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

    unsigned int entityId = m_PlayerDefinitions[i].EntityID;
    std::string eventString = package.PopFrontString();
    if ("+Forward" == eventString) {
        m_World->GetComponent(entityId, "Player")["Forward"] = true;
        m_World->GetComponent(entityId, "Player")["Back"] = false;
    } else if ("-Forward" == eventString) {
        m_World->GetComponent(entityId, "Player")["Forward"] = false;
        m_World->GetComponent(entityId, "Player")["Back"] = true;
    } else if ("0Forward" == eventString) {
        m_World->GetComponent(entityId, "Player")["Forward"] = false;
        m_World->GetComponent(entityId, "Player")["Back"] = false;
    }
    if ("+Right" == eventString) {
        m_World->GetComponent(entityId, "Player")["Left"] = false;
        m_World->GetComponent(entityId, "Player")["Right"] = true;
    } else if ("-Right" == eventString) {
        m_World->GetComponent(entityId, "Player")["Right"] = false;
        m_World->GetComponent(entityId, "Player")["Left"] = true;
    } else if ("0Right" == eventString) {
        m_World->GetComponent(entityId, "Player")["Right"] = false;
        m_World->GetComponent(entityId, "Player")["Left"] = false;
    }
}

void Server::ParseConnect(Package& package)
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
            m_PlayersToCreate.push_back(i);

            m_PlayerDefinitions[i].Endpoint = m_ReceiverEndpoint;
            m_PlayerDefinitions[i].Name = package.PopFrontString();

            m_StopTimes[i] = std::clock();

            LOG_INFO("Player \"%s\" connected on IP: %s", m_PlayerDefinitions[i].Name, m_PlayerDefinitions[i].Endpoint.address().to_string());

            Package package(MessageType::Connect, m_SendPacketID);
            package.AddPrimitive<int>(i); // Player ID

            Send(package, i);

            // Send notification that a player has connected
            std::string str = m_PacketID + "Player " + m_PlayerDefinitions[i].Name + " connected on: "
                + m_PlayerDefinitions[i].Endpoint.address().to_string();
            Broadcast(str);
            break;
        }
    }
}

void Server::ParseDisconnect()
{
    LOG_INFO("%i: Parsing disconnect", m_PacketID);

    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            Disconnect(i);
            break;
        }
    }
}

void Server::ParseClientPing()
{
    LOG_INFO("%i: Parsing ping", m_PacketID);
    // Return ping
    Package package(MessageType::ClientPing, m_SendPacketID);
    package.AddString("Ping received");
    Send(package); // This dosen't work for multiple users
}

void Server::ParseServerPing()
{
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            m_StopTimes[i] = std::clock();
            break;
        }
    }
}

// NOT USED
void Server::ParseSnapshot(Package& package)
{
    // Does no logic. Returns snapshot if client request one
    // The snapshot is not a real snapshot tho...
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            m_Socket.send_to(
                boost::asio::buffer("I'm sending a snapshot to you guys!"),
                m_PlayerDefinitions[i].Endpoint,
                0);
        }
    }
}

void Server::IdentifyPacketLoss()
{
    // if no packets lost, difference should be equal to 1
    int difference = m_PacketID - m_PreviousPacketID;
    if (difference != 1) {
        LOG_INFO("%i Packet(s) were lost...", difference);
    }
}
