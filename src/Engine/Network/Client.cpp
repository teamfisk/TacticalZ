#include "Network/Client.h"

using namespace boost::asio::ip;


Client::Client() : m_Socket(m_IOService)
{
    m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address::from_string("192.168.1.2"), 13);
    // Set up network stream
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
    m_EInputCommand = decltype(m_EInputCommand)(std::bind(&Client::OnInputCommand, this, std::placeholders::_1));
    m_EventBroker->Subscribe(m_EInputCommand);

    LOG_INFO("Please enter your name: ");
    std::cin >> m_PlayerName;
    while (m_PlayerName.size() > 7) {
        LOG_INFO("Please enter your name (No longer than 7 characters):");
        std::cin >> m_PlayerName;
    }
    m_Socket.connect(m_ReceiverEndpoint);
    LOG_INFO("I am client. BIP BOP");
    ReadFromServer();
}

void Client::Update()
{
    while (m_PlayersToCreate.size() > 0) {
        unsigned int i = m_PlayersToCreate.size() - 1;
        unsigned int tempID = m_World->CreateEntity();
        ComponentWrapper transform = m_World->AttachComponent(tempID, "Transform");
        ComponentWrapper model = m_World->AttachComponent(tempID, "Model");
        model["Resource"] = "Models/Core/UnitSphere.obj";
        ComponentWrapper player = m_World->AttachComponent(tempID, "Player");
        m_PlayerDefinitions[m_PlayersToCreate[i]].EntityID = tempID;
        m_PlayersToCreate.pop_back();
    }
}

void Client::Close()
{
    if (m_WasStarted) {
        Disconnect();
        m_ThreadIsRunning = false;
        m_EventBroker->Unsubscribe(m_EInputCommand);
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
            if (IsConnected()) {
                SendSnapshotToServer();
            }
            previousSnapshotMessage = currentTime;
        }
    }
}

void Client::SendSnapshotToServer()
{
    // Reset previouse key state in snapshot.
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
        Package package(MessageType::Event, m_SendPacketID);
        package.AddString(m_NextSnapshot.InputForward);
        Send(package);
    } else {
        Package package(MessageType::Event, m_SendPacketID);
        package.AddString("0Forward");
        Send(package);
    }

    if (m_NextSnapshot.InputRight != "") {
        Package package(MessageType::Event, m_SendPacketID);
        package.AddString(m_NextSnapshot.InputRight);
        Send(package);
    } else {
        Package package(MessageType::Event, m_SendPacketID);
        package.AddString("0Right");
        Send(package);
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
    //IdentifyPacketLoss();

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
    m_PlayerID = package.PopFrontPrimitive<int>();
    LOG_INFO("%i: I am player: %i", m_PacketID, m_PlayerID);
}

void Client::ParsePing()
{
    m_DurationOfPingTime = 1000 * (std::clock() - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
    LOG_INFO("%i: response time with ctime(ms): %f", m_PacketID, m_DurationOfPingTime);
}

void Client::ParseServerPing()
{
    Package message(MessageType::ServerPing, m_SendPacketID);
    message.AddString("Ping recieved");
    Send(message);
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
        LOG_INFO("%i: Event message: %s", m_PacketID, command.c_str());
    }
}

void Client::ParseSnapshot(Package& package)
{
    std::string tempName;
    for (size_t i = 0; i < MAXCONNECTIONS; i++) {
        // We're checking for empty name for now. This might not be the best way,
        // but it is to avoid sending redundant data.
        tempName = package.PopFrontString();


        // Apply the position data read to the player entity
        // New player connected on the server side 
        if (m_PlayerDefinitions[i].Name == "" && tempName != "") {
            m_PlayerDefinitions[i].Name = tempName;
            m_PlayersToCreate.push_back(i);
        } else if (m_PlayerDefinitions[i].Name != "" && tempName == "") {
            // Someone disconnected
            // TODO: Insert code here
            break;
        } else if (m_PlayerDefinitions[i].Name == "" && tempName == "") {
            // Not a connected player
            break;
        }
        if (m_PlayerDefinitions[i].EntityID != -1) {

            // Move player to server position
            int dataSize = m_World->GetComponent(m_PlayerDefinitions[i].EntityID, "Transform").Info.Meta.Stride;
            memcpy(m_World->GetComponent(m_PlayerDefinitions[i].EntityID, "Transform").Data, package.PopData(dataSize), dataSize);
        }
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
        //LOG_ERROR("Receive: %s", error.message().c_str());
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

bool Client::OnInputCommand(const Events::InputCommand & e)
{
    if (IsConnected()) {
        ComponentWrapper& player = m_World->GetComponent(m_PlayerDefinitions[m_PlayerID].EntityID, "Player");
        if (e.Command == "Forward") {
            if (e.Value > 0) {
                (bool&)player["Forward"] = true;
                (bool&)player["Back"] = false;
            } else if (e.Value < 0) {
                (bool&)player["Back"] = true;
                (bool&)player["Forward"] = false;
            } else {
                (bool&)player["Forward"] = false;
                (bool&)player["Back"] = false;
            }
        }
        if (e.Command == "Right") {
            if (e.Value > 0) {
                (bool&)player["Right"] = true;
                (bool&)player["Left"] = false;
            } else if (e.Value < 0) {
                (bool&)player["Left"] = true;
                (bool&)player["Right"] = false;
            } else {
                (bool&)player["Left"] = false;
                (bool&)player["Right"] = false;
            }
        }
    }
    if (e.Command == "ConnectToServer") { // Connect for now
        Connect();
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

bool Client::IsConnected()
{
    if (m_PlayerID != -1) {
        if (m_PlayerDefinitions[m_PlayerID].EntityID != -1) {
            return true;
        }
    }
    return false;
}
