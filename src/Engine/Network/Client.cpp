#include "Network/Client.h"

using namespace boost::asio::ip;


Client::Client() : m_Socket(m_IOService)
{
    // Set up network stream
    m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address::from_string("192.168.1.6"), 13);
    m_NextSnapshot.InputForward = "";
    m_NextSnapshot.InputRight = "";
}

Client::~Client()
{
    // will it work on linux? #if defined (_WIN64) | defined(_WIN32) otherwise.
    _CrtDumpMemoryLeaks();
}

void Client::Start(World* world, EventBroker* eventBroker)
{
    m_WasStarted = true;
    m_EventBroker = eventBroker;
    m_World = world;

    // Subscribe to events
    m_EKeyDown = decltype(m_EKeyDown)(std::bind(&Client::OnKeyDown, this, std::placeholders::_1));
    m_EventBroker->Subscribe(m_EKeyDown);
    m_EKeyUp = decltype(m_EKeyUp)(std::bind(&Client::OnKeyUp, this, std::placeholders::_1));
    m_EventBroker->Subscribe(m_EKeyUp);

    std::cout << "Please enter you name: ";
    std::cin >> m_PlayerName;
    while (m_PlayerName.size() > 7) {
        std::cout << "Please enter you name(No longer than 7 characters): ";
        std::cin >> m_PlayerName;
    }
    m_Socket.connect(m_ReceiverEndpoint);
    std::cout << "I am client. BIP BOP\n";
    ReadFromServer();
}

void Client::Close()
{
    if (m_WasStarted) {
        Disconnect();
        m_ThreadIsRunning = false;
        m_EventBroker->Unsubscribe(m_EKeyDown);
        m_EventBroker->Unsubscribe(m_EKeyUp);
    }
}

void Client::ReadFromServer()
{
    int bytesRead = -1;
    char readBuf[1024] = { 0 };

    int snapshotInterval = 33;
    std::clock_t previousSnapshotMessage = std::clock();

    while (m_ThreadIsRunning) {
        if (m_Socket.available()) {
            bytesRead = Receive(readBuf, INPUTSIZE);
            if (bytesRead > 0) {
                Package package(readBuf, bytesRead);
                ParseMessageType(package);
            }
        }
        std::clock_t currentTime = std::clock();
        if (snapshotInterval < (1000 * (currentTime - previousSnapshotMessage) / (double)CLOCKS_PER_SEC)) {
            SendSnapshotToServer();
            previousSnapshotMessage = currentTime;
        }
    }
}

void Client::SendSnapshotToServer()
{
    // Reset previouse key state in snapshot.
    m_NextSnapshot.InputForward = "";
    m_NextSnapshot.InputRight = "";
    // See if any movement keys are down
    // We dont care if it's overwritten by later
    // if statement. Watcha gonna do, right!
    if (m_IsWASDKeyDown.W) {
        m_NextSnapshot.InputForward = "+Forward";
    }
    if (m_IsWASDKeyDown.A) {
        m_NextSnapshot.InputRight = "-Right";
    }
    if (m_IsWASDKeyDown.S) {
        m_NextSnapshot.InputForward = "-Forward";
    }
    if (m_IsWASDKeyDown.D) {
        m_NextSnapshot.InputRight = "+Right";
    }

    if (m_NextSnapshot.InputForward != "") {
        Package message(MessageType::Event, m_SendPacketID);
        message.AddString(m_NextSnapshot.InputForward);
        Send(message);
    }
    if (m_NextSnapshot.InputRight != "") {
        Package message(MessageType::Event, m_SendPacketID);
        message.AddString(m_NextSnapshot.InputRight);
        Send(message);
    }
}

void Client::ParseMessageType(Package& package)
{
    int messageType = package.PopFrontPrimitive<int>();
    if (messageType == -1)
        return;
    // Read packet ID 
    m_PreviousPacketID = m_PacketID;    // Set previous packet id
    m_PacketID = package.PopFrontPrimitive<int>(); //Read new packet id
    IdentifyPacketLoss();

    switch (static_cast<MessageType>(messageType)) {
    case MessageType::Connect:
        ParseConnect(package);
        break;
    case MessageType::ClientPing:
        ParsePing();
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
        break;
    case MessageType::Event:
        ParseEventMessage(package);
        break;
    default:
        break;
    }
}

void Client::ParseConnect(Package& package)
{
    m_PacketID = package.PopFrontPrimitive<int>();
    m_PreviousPacketID = m_PacketID;
    m_PlayerID = package.PopFrontPrimitive<int>();
    std::cout << m_PacketID << ": I am player: " << m_PlayerID << std::endl;
}

void Client::ParsePing()
{
    m_DurationOfPingTime = 1000 * (std::clock() - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
    std::cout << m_PacketID << ": response time with ctime(ms): " << m_DurationOfPingTime << std::endl;
}

void Client::ParseServerPing()
{
    Package message(MessageType::ServerPing, m_SendPacketID);
    message.AddString("Ping recieved");
    Send(message);
    //std::cout << "Parsing ping." << std::endl;
}

void Client::ParseEventMessage(Package& package)
{
    int Id = -1;
    std::string command = package.PopFrontString();
    if (command.find("+Player") != std::string::npos) {
        Id = package.PopFrontPrimitive<int>();
        // Sett Player name
        m_PlayerDefinitions[Id].Name = command.erase(0, 7);
    } else {
        std::cout << m_PacketID << ": Event message: " << command << std::endl;
    }
}

void Client::ParseSnapshot(Package& package)
{
    //std::cout << m_PacketID << ": Parsing incoming snapshot." << std::endl;
    std::string tempName;
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        // We're checking for empty name for now. This might not be the best way,
        // but it is to avoid sending redundant data.
        tempName = package.PopFrontString();
        
        // Apply the position data read to the player entity
        // New player connected on the server side 
        if (m_PlayerDefinitions[i].Name == "" && tempName != "") {
            CreateNewPlayer(i);
        } else if (m_PlayerDefinitions[i].Name != "" && tempName == "") {
            // Someone disconnected
            // TODO: Insert code here
            break;
        } else if (m_PlayerDefinitions[i].Name == "" && tempName == "") {
            // Not a connected player
            break;
        }
        // Read position data
        glm::vec3 playerPos;
        playerPos.x = package.PopFrontPrimitive<float>();
        playerPos.y = package.PopFrontPrimitive<float>();
        playerPos.z = package.PopFrontPrimitive<float>();
        // Move player to server position
        m_World->GetComponent(m_PlayerDefinitions[i].EntityID, "Transform")["Position"] = playerPos;
        m_PlayerDefinitions[i].Name = tempName;
    }
}

int Client::Receive(char* data, size_t length)
{
    boost::system::error_code error;

    int bytesReceived = m_Socket.receive_from(boost
        ::asio::buffer((void*)data, length),
        m_ReceiverEndpoint,
        0, error);

    if (error) {
        std::cout << "ReadFromServer crashed: " << error.message();
    }

    return bytesReceived;
}

void Client::Send(Package& package)
{
    m_Socket.send_to(boost::asio::buffer(
        package.Data(),
        package.Size()),
        m_ReceiverEndpoint, 0);
}

void Client::Connect()
{
    Package message(MessageType::Connect, m_SendPacketID);
    message.AddString(m_PlayerName);
    m_StartPingTime = std::clock();
    Send(message);
}

void Client::Disconnect()
{
    Package message(MessageType::Connect, m_SendPacketID);
    message.AddString("+Disconnect");
    Send(message);
}

void Client::Ping()
{
    Package message(MessageType::Connect, m_SendPacketID);
    message.AddString("Ping");
    m_StartPingTime = std::clock();
    Send(message);
}

void Client::MoveMessageHead(char*& data, size_t& length, size_t stepSize)
{
    data += stepSize;
    length -= stepSize;
}

bool Client::OnKeyDown(const Events::KeyDown& event)
{
    if (event.KeyCode == GLFW_KEY_W) {
        m_IsWASDKeyDown.W = true;
    }
    if (event.KeyCode == GLFW_KEY_A) {
        m_IsWASDKeyDown.A = true;
    }
    if (event.KeyCode == GLFW_KEY_S) {
        m_IsWASDKeyDown.S = true;
    }
    if (event.KeyCode == GLFW_KEY_D) {
        m_IsWASDKeyDown.D = true;
    }

    if (event.KeyCode == GLFW_KEY_V) {
        Disconnect();
    }
    if (event.KeyCode == GLFW_KEY_C) {
        Connect();
    }
    if (event.KeyCode == GLFW_KEY_P) {
        Ping();
    }
    return true;
}

bool Client::OnKeyUp(const Events::KeyUp & e)
{
    if (e.KeyCode == GLFW_KEY_W) {
        m_IsWASDKeyDown.W = false;
        return true;
    }
    if (e.KeyCode == GLFW_KEY_A) {
        m_IsWASDKeyDown.A = false;
        return true;
    }
    if (e.KeyCode == GLFW_KEY_S) {
        m_IsWASDKeyDown.S = false;
        return true;
    }
    if (e.KeyCode == GLFW_KEY_D) {
        m_IsWASDKeyDown.D = false;
        return true;
    }
    return false;
}

void Client::CreateNewPlayer(int i)
{
    m_PlayerDefinitions[i].EntityID = m_World->CreateEntity();
    ComponentWrapper transform = m_World->AttachComponent(m_PlayerDefinitions[i].EntityID, "Transform");
    ComponentWrapper model = m_World->AttachComponent(m_PlayerDefinitions[i].EntityID, "Model");
    model["Resource"] = "Models/Core/UnitSphere.obj";
}

void Client::IdentifyPacketLoss()
{
    // if no packets lost, difference should be equal to 1
    int difference = m_PacketID - m_PreviousPacketID;
    if (difference != 1) {
			LOG_INFO("%i Packet(s) were lost...", difference);
    }
}
