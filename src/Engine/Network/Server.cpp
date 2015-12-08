#include "Network/Server.h"

Server::Server() : m_Socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 13))
{ 
}

Server::~Server()
{
}


void Server::Start(World* world)
{
    m_World = world;
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        m_StopTimes[i] = std::clock();
        m_PlayerPositions[i].x = -1;
        m_PlayerPositions[i].y = -1;
    }
    m_PlayerPositions[0].x = 0;
    m_PlayerPositions[0].y = 0;
    boost::thread_group threads;

    std::cout << "I am Server. BIP BOP\n";

    threads.create_thread(boost::bind(&Server::DisplayLoop, this));
    threads.create_thread(boost::bind(&Server::ReadFromClients, this));
    threads.create_thread(boost::bind(&Server::InputLoop, this));

    threads.join_all();
}

void Server::DisplayLoop()
{
    int lengthOfMsg = -1;
    std::clock_t previousePingMessage = std::clock();
    std::clock_t previousSnapshotMessage = std::clock();
    std::clock_t timOutTimer = std::clock();
    int intervallMs = 1000;
    int snapshotInterval = 50;
    int timeToCheckTimeOutTime = 100;
    char* data;

    for (;;) {

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

void Server::ReadFromClients()
{
    char readBuf[1024] = { 0 };
    int bytesRead = 0;

    for (;;) {
        if (m_Socket.available()) {
            try {
                bytesRead = Receive(readBuf, INPUTSIZE);
                ParseMessageType(readBuf, bytesRead);
            } catch (const std::exception& err) {
                // To not spam "socket closed messages"
                //if (std::string(err.what()).find("forcefully closed") != std::string::npos) {
                std::cout << "Read from client crashed: " << err.what();
                //}
            }
        }
    }
}

void Server::InputLoop()
{
    char inputBuffer[INPUTSIZE] = { 0 };
    std::string inputMessage;

    for (;;) {
        std::cin.getline(inputBuffer, INPUTSIZE);
        inputMessage = (std::string)inputBuffer;

        if (!inputMessage.empty()) {
            try {
                // Broadcast message typed in console 
                Broadcast(inputMessage);

            } catch (const std::exception& err) {
                std::cout << "Read from WriteLoop crashed: " << err.what();
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

    switch (static_cast<MessageType>(messageType)) {
    case MessageType::Connect:
        ParseConnect(data, length);
        break;
    case MessageType::ClientPing:
        ParseClientPing();
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

int Server::CreateMessage(MessageType type, std::string message, char * data)
{
    int lengthOfMessage = 0;
    int offset = 0;

    lengthOfMessage = message.size();
    // Message type
    memcpy(data + offset, &type, sizeof(int));
    offset += sizeof(int);
    // Message, add one extra byte for null terminator
    memcpy(data + offset, message.data(), (lengthOfMessage + 1) * sizeof(char));
    offset += (lengthOfMessage + 1) * sizeof(char);

    return offset;
}

void Server::MoveMessageHead(char *& data, size_t & length, size_t stepSize)
{
    data += stepSize;
    length -= stepSize;
}

void Server::Broadcast(std::string message)
{
    std::cout << "Broadcast: " << message << std::endl;
    char* data = new char[128];
    int offset = CreateMessage(MessageType::Event, message, data);
    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            m_Socket.send_to(
                boost::asio::buffer(data, offset),
                m_PlayerDefinitions[i].Endpoint,
                0);
        }
    }
    delete[] data;
}

void Server::Broadcast(char * data, size_t length)
{
    for (int i = 0; i < MAXCONNECTIONS; ++i) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address()) {
            m_Socket.send_to(
                boost::asio::buffer(data, length),
                m_PlayerDefinitions[i].Endpoint,
                0);
        }
    }
}

void Server::SendSnapshot()
{
    char* data = new char[128];
    int offset = CreateHeader(MessageType::Snapshot, data);
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        memcpy(data + offset, &m_PlayerPositions[i].x, sizeof(float));
        offset += sizeof(float);
        memcpy(data + offset, &m_PlayerPositions[i].y, sizeof(float));
        offset += sizeof(float);
        // +1 for null terminator
        memcpy(data + offset, m_PlayerDefinitions[i].Name.data(), m_PlayerDefinitions[i].Name.size() + 1);
        offset += (m_PlayerDefinitions[i].Name.size() + 1) * sizeof(char);
    }
    Broadcast(data, offset);
    delete[] data;
}

void Server::SendPing()
{
    // Prints connected players ping
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() != boost::asio::ip::address())
            std::cout << "Player " << i << "'s ping: " << 1000 * (m_StopTimes[i] - m_StartPingTime)
            / static_cast<double>(CLOCKS_PER_SEC) << std::endl;
    }

    // Create ping message
    char* data = new char[128];
    int len = CreateMessage(MessageType::ServerPing, "Ping from server", data);
    // Time message
    m_StartPingTime = std::clock();
    // Send message
    Broadcast(data, len);
    delete[] data;

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

int Server::CreateHeader(MessageType type, char * data)
{
    int messageType = static_cast<int>(type);
    int offset = 0;
    memcpy(data, &messageType, sizeof(int));
    offset += sizeof(int);

    return offset;
}

void Server::Disconnect(int i)
{
    Broadcast("A player disconnected");
    std::cout << "Player " << i << " disconnected/Timed out" << std::endl;
    m_PlayerDefinitions[i].Endpoint = boost::asio::ip::udp::endpoint();
    m_PlayerDefinitions[i].EntityID = -1;
    m_PlayerDefinitions[i].Name = "Name not Set";
    // Reset disconnected players position
    m_PlayerPositions[i].x = -1;
    m_PlayerPositions[i].y = -1;
}

void Server::ParseEvent(char * data, size_t length)
{
    size_t i;
    for (i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            break;
        }
    }
    // If no player matches the ip return.
    if (i >= 8)
        return;

    unsigned int entityId = m_PlayerDefinitions[i].EntityID;
    if ("+Forward" == std::string(data)) {
        glm::vec3 temp = m_World->GetComponent(entityId, "Transform")["Position"];
        temp.z -= 0.5f;
        m_World->GetComponent(entityId, "Transform")["Position"] = temp;
    }
    if ("-Forward" == std::string(data)) {
        glm::vec3 temp = m_World->GetComponent(entityId, "Transform")["Position"];
        temp.z += 0.5f;
        m_World->GetComponent(entityId, "Transform")["Position"] = temp;
    }
       
    if ("+Right" == std::string(data)) {
        glm::vec3 temp = m_World->GetComponent(entityId, "Transform")["Position"];
        temp.x += 0.5f;
        m_World->GetComponent(entityId, "Transform")["Position"] = temp;
    }
     
    if ("-Right" == std::string(data)) {
        glm::vec3 temp = m_World->GetComponent(entityId, "Transform")["Position"];
        temp.x -= 0.5f;
        m_World->GetComponent(entityId, "Transform")["Position"] = temp;
    }
}

void Server::ParseConnect(char * data, size_t length)
{
    std::cout << "Parsing connection." << std::endl;

    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            return;
        }
    }

    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == boost::asio::ip::address()) {
            

            m_PlayerDefinitions[i].EntityID = m_World->CreateEntity();
            ComponentWrapper transform = m_World->AttachComponent(m_PlayerDefinitions[i].EntityID, "Transform");
            transform["Position"] = glm::vec3(-1.5f, 0.f, 0.f);
            ComponentWrapper model = m_World->AttachComponent(m_PlayerDefinitions[i].EntityID, "Model");
            model["Resource"] = "Models/Core/UnitSphere.obj";

            m_PlayerDefinitions[i].Endpoint = m_ReceiverEndpoint;
            m_PlayerDefinitions[i].Name = std::string(data);
            m_StopTimes[i] = std::clock();

            std::cout << "Player \"" << m_PlayerDefinitions[i].Name << "\" connected on IP: " << 
                m_PlayerDefinitions[i].Endpoint.address().to_string() << std::endl;

            int offset = 0;
            char* temp = new char[sizeof(int) * 2];
            int messagType = 0;

            memcpy(temp, &messagType, sizeof(int));
            offset += sizeof(int);
            memcpy(temp + offset, &i, sizeof(int));

            m_Socket.send_to(
                boost::asio::buffer(temp, sizeof(int) * 2),
                m_PlayerDefinitions[i].Endpoint,
                0);

            // Send notification that a player has connected
            std::string str = "Player " + m_PlayerDefinitions[i].Name + " connected on: " 
                + m_PlayerDefinitions[i].Endpoint.address().to_string();
            Broadcast(str);
            // +1 is the null terminator
            MoveMessageHead(data, length, m_PlayerDefinitions[i].Name.size() + 1);
            delete[] temp;
            break;
        }
    }
}

void Server::ParseDisconnect()
{
    std::cout << "Parsing disconnect. \n";

    for (int i = 0; i < MAXCONNECTIONS; i++) {
        if (m_PlayerDefinitions[i].Endpoint.address() == m_ReceiverEndpoint.address()) {
            Disconnect(i);
            break;
        }
    }
}

void Server::ParseClientPing()
{
    char* testMesssage = new char[128];
    int testOffset = CreateMessage(MessageType::ClientPing, "Ping recieved", testMesssage);

    std::cout << "Parsing ping." << std::endl;
    // Return ping
    m_Socket.send_to(
        boost::asio::buffer(
            testMesssage,
            testOffset),
        m_ReceiverEndpoint,
        0);
    delete[] testMesssage;
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