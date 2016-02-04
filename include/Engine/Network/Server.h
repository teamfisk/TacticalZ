#ifndef Server_h__
#define Server_h__

#include <string>
#include <ctime>

#include <glm/common.hpp>

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
protected:
    template<class T>
    T m_ReceiverEndpoint;
    // Sending messages to client logic
    std::map<PlayerID, PlayerDefinition> m_ConnectedPlayers;
    // HACK: Fix INPUTSIZE
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
    int m_NextPlayerID = 0;
    //Timers
    std::clock_t m_StartPingTime;

    // Game logic
    World* m_World;
    EventBroker* m_EventBroker;
    
    // Packet loss logic
    PacketID m_PacketID = 0;
    PacketID m_PreviousPacketID = 0;

    // Private member functions
    //int  receive(char* data);
    void broadcast(Packet& packet);
    void sendSnapshot();
    void addChildrenToPacket(Packet& packet, EntityID entityID);
    void sendPing();
    void checkForTimeOuts();
    void disconnect(PlayerID playerID);
    void parseMessageType(Packet& packet);
    void parseOnPlayerDamage(Packet& packet);
    void identifyPacketLoss();
    void kick(PlayerID player);
    // Pure virtual functions
    virtual void parseOnInputCommand(Packet& packet) = 0;
    virtual void readFromClients() = 0;
    virtual void send(Packet& packet, PlayerDefinition & playerDefinition) = 0;
    virtual void send(Packet& packet) = 0;
    virtual void parseConnect(Packet& packet) = 0;
    virtual void parseDisconnect() = 0;
    virtual void parseClientPing() = 0;
    virtual void parsePing() = 0;
    // Debug event
    EventRelay<Server, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<Server, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(const Events::PlayerSpawned& e);
    EventRelay<Server, Events::EntityDeleted> m_EEntityDeleted;
    bool OnEntityDeleted(const Events::EntityDeleted& e);
    EventRelay<Server, Events::ComponentDeleted> m_EComponentDeleted;
    bool OnComponentDeleted(const Events::ComponentDeleted& e);
    virtual void parsePlayerTransform(Packet& packet) = 0;
};

#endif

