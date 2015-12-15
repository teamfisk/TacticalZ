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
    boost::thread_group threads;

    std::cout << "I am Server. BIP BOP\n";

    threads.create_thread(boost::bind(&Server::DisplayLoop, this));
    threads.create_thread(boost::bind(&Server::ReadFromClients, this));
    threads.create_thread(boost::bind(&Server::InputLoop, this));

    threads.join_all();
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

void Server::DisplayLoop()
{

}

void Server::ReadFromClients()
{
    char readBuf[1024] = { 0 };
    int bytesRead = 0;
    // time for previouse message
    std::clock_t previousePingMessage = std::clock();
    std::clock_t previousSnapshotMessage = std::clock();
    std::clock_t timOutTimer = std::clock();
    // How often we send messages (milliseconds)
    int intervallMs = 1000;
    int snapshotInterval = 50;
    int timeToCheckTimeOutTime = 100;

    while (m_ThreadIsRunning) {
        // m_ThreadIsRunning might be unnecessary but the 
        // program crashed if it executed m_Socket.available()
        // when closing the program. 

        // If available message -> Socket.available() = true
        if (m_ThreadIsRunning && m_Socket.available()) {
            try {
                bytesRead = Receive(readBuf, INPUTSIZE);
                ParseMessageType(readBuf, bytesRead);
            } catch (const std::exception& err) {
                // To not spam "socket closed messages"
                //if (std::string(err.what()).find("forcefully closed") != std::string::npos) {
                std::cout << m_PacketID << ": Read from client crashed: " << err.what();
                //}
            }
            
        }
        std::clock_t currentTime = std::clock();
        // int tempTestRemovePlz = (1000 * (currentTime - previousSnapshotMessage) / (double)CLOCKS_PER_SEC);
        // Send snapshot
        if (snapshotInterval < (1000 * (currentTime - previousSnapshotMessage) / (double)CLOCKS_PER_SEC)) {
            SendSnapshot();
            previousSnapshotMessage = currentTime;
        }

        // Send pings each 
        if (intervallMs < (1000 * (currentTime - previousePingMessage) / (double)CLOCKS_PER_SEC)) {
            SendPing();
            previousePingMessage = currentTime;
        }

        // Time out logic
        if (timeToCheckTimeOutTime < (1000 * (currentTime - timOutTimer) / (double)CLOCKS_PER_SEC)) {
            CheckForTimeOuts();
            timOutTimer = currentTime;
        }
    }
}

void Server::InputLoop()
{
    char inputBuffer[INPUTSIZE] = { 0 };
    std::string inputMessage;

    while (m_ThreadIsRunning) {
        std::cin.getline(inputBuffer, INPUTSIZE);
        inputMessage = (std::string)inputBuffer;

        if (!inputMessage.empty()) {
            try {
                // Broadcast message typed in console 
                Broadcast(inputMessage);

            } catch (const std::exception& err) {
                std::cout << m_PacketID << ": Read from WriteLoop crashed: " << err.what();
            }
        }
        if (inputMessage.find("exit") != std::string::npos)
            exit(1);
        inputMessage.clear();
        memset(inputBuffer, 0, INPUTSIZE);
    }
}

void Server::ParseMessageType(char * data, size_t length)
{
    int messageType = -1;
    memcpy(&messageType, data, sizeof(int)); // Read what type off message was sent from server
    MoveMessageHead(data, length, sizeof(int)); // Move the message head to know where to read from

    // Read packet ID 
    m_PreviousPacketID = m_PacketID;    // Set previous packet id
    memcpy(&m_PacketID, data, sizeof(int)); //Read new packet id
    MoveMessageHead(data, length, sizeof(int));
    //IdentifyPacketLoss(); 
    switch (static_cast<MessageType>(messageType)) {
    case MessageType::Connect:
        ParseConnect(data, length);
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
        ParseSnapshot(data, length);
        break;
    case MessageType::Disconnect:
        ParseDisconnect();
        break;
    case MessageType::Event:
        ParseEvent(data, length);
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
        if (m_PlayerDefinitions[i].EntityID == -1) {
            continue;
        }


        // Pack player pos into data package
        glm::vec3 playerPos = m_World->GetComponent(m_PlayerDefinitions[i].EntityID, "Transform")["Position"];
        //glm::vec3 playerPos = glm::vec3(1.0f);
        package.AddPrimitive<float>(playerPos.x);
        package.AddPrimitive<float>(playerPos.y);
        package.AddPrimitive<float>(playerPos.z);

        package.AddString(m_PlayerDefinitions[i].Name);
    }
    Broadcast(package);
}

void Server::SendPing()
{
    // Prints connected players ping
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address())
            std::cout << m_PacketID << ": Player " << i << "'s ping: " << 1000 * (m_StopTimes[i] - m_StartPingTime)
            / static_cast<double>(CLOCKS_PER_SEC) << std::endl;
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
    int tempStartPing = 1000 * m_StartPingTime
        / static_cast<double>(CLOCKS_PER_SEC);

    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            int tempStopPing = 1000 * m_StopTimes[i]
                / static_cast<double>(CLOCKS_PER_SEC);
            if (tempStartPing > tempStopPing + timeOutTimeMs) {
                std::cout << "player " << i << " timed out!" << std::endl;
                Disconnect(i);
            }
        }
    }
}

void Server::Disconnect(int i)
{
    Broadcast("A player disconnected");
    std::cout << "Player " << i << " disconnected/Timed out" << std::endl;

    // Remove enteties and stuff
    m_PlayerDefinitions[i].Endpoint = boost::asio::ip::udp::endpoint();
    m_PlayerDefinitions[i].EntityID = -1;
    m_PlayerDefinitions[i].Name = "";
}

void Server::ParseEvent(char * data, size_t length)
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
    m_World->GetComponent(entityId, "Player")["Forward"] = false;
    m_World->GetComponent(entityId, "Player")["Left"] = false;
    m_World->GetComponent(entityId, "Player")["Back"] = false;
    m_World->GetComponent(entityId, "Player")["Right"] = false;

    if ("+Forward" == std::string(data)) {
        m_World->GetComponent(entityId, "Player")["Forward"] = true;
    }
    if ("-Forward" == std::string(data)) {
        m_World->GetComponent(entityId, "Player")["Back"] = true;
    }
    if ("+Right" == std::string(data)) {
        m_World->GetComponent(entityId, "Player")["Right"] = true;
    }
    if ("-Right" == std::string(data)) {
        m_World->GetComponent(entityId, "Player")["Left"] = true;
    }
}

void Server::ParseConnect(char * data, size_t length)
{
    std::cout << "Parsing connection." << std::endl;
    // Check if player is already connected
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            return;
        }
    }

    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == boost::asio::ip::address()) {
            
            //Events::CreatePlayer e;
            //e.entityID = (m_PlayerDefinitions[i].EntityID);
            //e.modelPath = "Models/Core/UnitSphere.obj";
            //e.world = m_World;
            //m_EventBroker->Publish(e);

            // Create new player
            m_PlayersToCreate.push_back(i);

            m_PlayerDefinitions[i].Endpoint = m_ReceiverEndpoint;
            m_PlayerDefinitions[i].Name = std::string(data);
            // +1 is the null terminator
            MoveMessageHead(data, length, m_PlayerDefinitions[i].Name.size() + 1);
            m_StopTimes[i] = std::clock();

            std::cout << m_PacketID << ": Player \"" << m_PlayerDefinitions[i].Name << "\" connected on IP: " <<
                m_PlayerDefinitions[i].Endpoint.address().to_string() << std::endl;

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
    std::cout << m_PacketID << ":Parsing disconnect. \n";

    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            Disconnect(i);
            break;
        }
    }
}

void Server::ParseClientPing()
{
    std::cout << m_PacketID << ":Parsing ping." << std::endl;
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
void Server::ParseSnapshot(char * data, size_t length)
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
