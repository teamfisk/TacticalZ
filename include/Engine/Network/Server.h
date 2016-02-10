#ifndef Server_h__
#define Server_h__

#include <string>
#include <ctime>

#include <glm/common.hpp>

#include "Network/TCPServer.h"
#include "Network/UDPServer.h"
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
    // dont forget to set these in the childrens receive logic
    boost::asio::ip::address m_Address;
    unsigned short m_Port;
    // Sending messages to client logic
    std::map<PlayerID, PlayerDefinition> m_ConnectedPlayers;
    // HACK: Fix INPUTSIZE
    char readBuffer[BUFFERSIZE] = { 0 };
    size_t bytesRead = 0;
    // time for previouse message
    std::clock_t previousePingMessage = std::clock();
    std::clock_t previousSnapshotMessage = std::clock();
    std::clock_t timOutTimer = std::clock();

    // How often we send messages (milliseconds)
    float pingIntervalMs;
    float snapshotInterval;
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
    void reliableBroadcast(Packet& packet);
    void unreliableBroadcast(Packet& packet);
    void sendSnapshot();
    void addChildrenToPacket(Packet& packet, EntityID entityID);
    void sendPing();
    void checkForTimeOuts();
    void disconnect(PlayerID playerID);
    void parseMessageType(Packet& packet);
    void parseOnPlayerDamage(Packet& packet);
    void identifyPacketLoss();
    void kick(PlayerID player);
    PlayerID GetPlayerIDFromEndpoint();
    void parsePlayerTransform(Packet& packet);
    void parseOnInputCommand(Packet& packet);
    void parseClientPing();
    void parsePing();    
    void parseUDPConnect(Packet & packet);
    void parseTCPConnect(Packet & packet);
    void parseDisconnect();

    // Debug event
    EventRelay<Server, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<Server, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(const Events::PlayerSpawned& e);
    EventRelay<Server, Events::EntityDeleted> m_EEntityDeleted;
    bool OnEntityDeleted(const Events::EntityDeleted& e);
    EventRelay<Server, Events::ComponentDeleted> m_EComponentDeleted;
    bool OnComponentDeleted(const Events::ComponentDeleted& e);
private:
    TCPServer m_Reliable;
    UDPServer m_Unreliable;
};

#endif

