#ifndef Client_h__
#define Client_h__

#include <string>
#include <ctime>

#include <glm/common.hpp>

#include "Network/MessageType.h"
#include "Network/NetworkDefinitions.h"
#include "Network/PlayerDefinition.h"
#include "Network/SnapshotDefinitions.h"
#include "Network/WinLeakCheck.h"
#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Input/EInputCommand.h"
#include "Network/Network.h"

class Client : public Network
{
public:
    Client();
    ~Client();
    void Start(World* world, EventBroker* eventBroker);
    void Update();
    void Close();
private:
    void ReadFromServer();
    void SendSnapshotToServer();

    int Receive(char* data, size_t length);
    void Send(Package& message);
    void Connect();
    void Disconnect();
    void Ping();
    void MoveMessageHead(char*& data, size_t& length, size_t stepSize);
    void ParseMessageType(Package& package);
    void ParseEventMessage(Package& package);
    void ParseConnect(Package& package);
    void ParsePing();
    void ParseServerPing();
    void ParseSnapshot(Package& package);
    void CreateNewPlayer(int i);
    void IdentifyPacketLoss();

    // UDP logic
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::socket m_Socket;

    // Packet loss logic
    unsigned int m_PacketID = 0;
    unsigned int m_PreviousPacketID = 0;
    unsigned int m_SendPacketID = 0;

    // Game logic
    World* m_World;
    std::vector<unsigned int> m_PlayersToCreate;
    glm::vec2 m_PlayerPositions[MAXCONNECTIONS];
    std::string m_PlayerName;
    int m_PlayerID = -1;

    // Network logic
    PlayerDefinition m_PlayerDefinitions[MAXCONNECTIONS];
    SnapshotDefinitions m_NextSnapshot;
    bool m_ThreadIsRunning = true;
    double m_DurationOfPingTime;
    std::clock_t m_StartPingTime;
    // Use to check if we should send disconnect message
    // if game is turned of by closing window.
    bool m_WasStarted = false;

    // Events
    EventBroker* m_EventBroker;
    EventRelay<Client, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand &e);
};

#endif

