#ifndef Server_h__
#define Server_h__

#include <string>
#include <ctime>

#include <glm/common.hpp>
#include <boost/asio/ip/udp.hpp>

#include "Network/MessageType.h"
#include "Network/PlayerDefinition.h"
#include "Core/World.h"
#include "Core/EventBroker.h"
#include "../Network/Network.h"
#include "Input/EInputCommand.h"
#include "Core/EPlayerDamage.h"
#include "Network/EPlayerDisconnected.h"
#include "Core/EPlayerSpawned.h"
#include "Core/EEntityDeleted.h"
#include "Core/EComponentDeleted.h"

class Server : public Network
{
public:
    Server();
    ~Server();
    void Start(World* m_world, EventBroker *eventBroker) override;
    void Update() override;
private:
    // UDP logic
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::socket m_Socket;

    // Sending messages to client logic
    PlayerDefinition m_PlayerDefinitions[8]; // 
    std::vector<PlayerDefinition> m_ConnectedUsers;
    char readBuffer[INPUTSIZE] = { 0 };
    int bytesRead = 0;
    // time for previouse message
    std::clock_t previousePingMessage = std::clock();
    std::clock_t previousSnapshotMessage = std::clock();
    std::clock_t timOutTimer = std::clock();
    // How often we send messages (milliseconds)
    int pingIntervalMs;
    int snapshotInterval;
    int checkTimeOutInterval = 100;

    //Timers
    std::clock_t m_StartPingTime;

    // Game logic
    World* m_World;
    EventBroker* m_EventBroker;
    
    // Packet loss logic
    PacketID m_PacketID = 0;
    PacketID m_PreviousPacketID = 0;

    // Private member functions
    int  receive(char* data);
    void readFromClients();
    void send(Packet& packet, UserID user);
    void send(PlayerID player, Packet& packet);
    void send(Packet& packet);
    void broadcast(Packet& packet);
    void sendSnapshot();
    void addChildrenToPacket(Packet& packet, EntityID entityID);
    void sendPing();
    void checkForTimeOuts();
    void disconnect(UserID user);
    void parseMessageType(Packet& packet);
    void parseOnInputCommand(Packet& packet);
    void parseOnPlayerDamage(Packet& packet);
    void parseConnect(Packet& packet);
    void parseDisconnect();
    void parseClientPing();
    void parsePing();
    void identifyPacketLoss();
    void createPlayer();
    void kick(PlayerID player);
    PlayerID GetPlayerIDFromEndpoint(boost::asio::ip::udp::endpoint endpoint);
    // Debug event
    EventRelay<Server, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<Server, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(const Events::PlayerSpawned& e);
    EventRelay<Server, Events::EntityDeleted> m_EEntityDeleted;
    bool OnEntityDeleted(const Events::EntityDeleted& e);
    EventRelay<Server, Events::ComponentDeleted> m_EComponentDeleted;
    bool OnComponentDeleted(const Events::ComponentDeleted& e);
    void parsePlayerTransform(Packet& packet);
};

#endif

