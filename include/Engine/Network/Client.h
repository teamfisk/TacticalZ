#ifndef Client_h__
#define Client_h__

#include <string>
#include <ctime>

#include <glm/common.hpp>

#include "Network/Network.h"
#include "Network/MessageType.h"
#include "Network/NetworkDefinitions.h"
#include "Network/PlayerDefinition.h"
#include "Network/SnapshotDefinitions.h"
#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Core/ConfigFile.h"
#include "Input/EInputCommand.h"

class Client : public Network
{
public:
    Client(ConfigFile* config);
    ~Client();
    void Start(World* world, EventBroker* eventBroker);
    void Update();
    void Close();
private:
    // Assio UDP logic
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

    // Private member functions
    void readFromServer();
    void sendSnapshotToServer();
    int receive(char* data, size_t length);
    void send(Packet& packet);
    void connect();
    void disconnect();
    void ping();
    void moveMessageHead(char*& data, size_t& length, size_t stepSize);
    void parseMessageType(Packet& packet);
    void parseEventMessage(Packet& packet);
    void parseConnect(Packet& packet);
    void parsePing();
    void parseServerPing();
    void parseSnapshot(Packet& packet);
    void createNewPlayer(int i);
    void identifyPacketLoss();
    bool isConnected();

    // Events
    EventBroker* m_EventBroker;
    EventRelay<Client, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand &e);
};

#endif

