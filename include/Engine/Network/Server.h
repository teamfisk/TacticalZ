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
#include "Core/EntityFile.h"
#include "Core/EventBroker.h"
#include "../Network/Network.h"
#include "Input/EInputCommand.h"
#include "Core/EPlayerDamage.h"
#include "Network/EPlayerDisconnected.h"
#include "Core/EPlayerSpawned.h"
#include "../Game/Events/EDoubleJump.h"
#include "Core/EEntityDeleted.h"
#include "Core/EComponentDeleted.h"
#include "Core/EAmmoPickup.h"
#include "Core/EPlayerDeath.h"
#include "Network/EPlayerConnected.h"
#include "Network/EKillDeath.h"
#include "Core/EWin.h"
#include "Game/Events/EReset.h"

class Server : public Network
{
public:
    Server(World* world, EventBroker* eventBroker, int port);
    ~Server();

    void Update(double dt) override;

private:
    // Network channels
    TCPServer m_Reliable;
    UDPServer m_Unreliable;
    UDPServer m_ServerlistRequest;
    // dont forget to set these in the childrens receive logic
    boost::asio::ip::address m_Address;
    int m_Port = 27666;
    bool m_GameIsOver = false;
    // Sending messages to client logic
    std::map<PlayerID, PlayerDefinition> m_ConnectedPlayers;
    std::vector<PlayerID> m_PlayersToDisconnect;
    // HACK: Fix INPUTSIZE
    char readBuffer[BUFFERSIZE] = { 0 };
    size_t bytesRead = 0;
    // time for previouse message
    double previousPingMessage = 0;
    double previousSnapshotMessage = 0;
    double timeOutTimer = 0;

    // How often we send messages (seconds)
    double pingInterval = 1;
    double snapshotInterval = 0.05;
    double checkTimeOutInterval = 0.1;
    int m_NextPlayerID = 0;
    std::vector<Events::InputCommand> m_InputCommandsToBroadcast;
    //Timers
    std::clock_t m_StartPingTime;
    std::string m_ServerName = "";

    // Packet loss logic
    PacketID m_PacketID = 0;
    PacketID m_PreviousPacketID = 0;

    // Private member functions
    //int  receive(char* data);
    void reliableBroadcast(Packet& packet);
    void unreliableBroadcast(Packet& packet);
    void sendSnapshot();
    void createWorldSnapshot(Packet& packet);
    void addPlayersToPacket(Packet& packet, EntityID entityID);
    void addChildrenToPacket(Packet& packet, EntityID entityID);
    void addInputCommandsToPacket(Packet& packet);
    void sendPing();
    void checkForTimeOuts();
    void disconnect(PlayerID playerID);
    void parseMessageType(Packet& packet);
    void parseOnPlayerDamage(Packet& packet);
    void identifyPacketLoss();
    void kick(PlayerID player);
    PlayerID getPlayerIDFromEndpoint();
    PlayerID getPlayerIDFromEntityID(EntityID entityID);
    void resetMap();
    void parsePlayerTransform(Packet& packet);
    void parseOnInputCommand(Packet& packet);
    void parseClientPing();
    void parsePing();
    bool parseDoubleJump(Packet& packet);
    void parseDashEffect(Packet& packet);
    void parseUDPConnect(Packet& packet);
    void parseTCPConnect(Packet& packet);
    void parseDisconnect();
    void parseServerlistRequest(boost::asio::ip::udp::endpoint endpoint);
    bool shouldSendToClient(EntityWrapper childEntity);

    // Events
    EventRelay<Server, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<Server, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(const Events::PlayerSpawned& e);
    EventRelay<Server, Events::EntityDeleted> m_EEntityDeleted;
    bool OnEntityDeleted(const Events::EntityDeleted& e);
    EventRelay<Server, Events::ComponentDeleted> m_EComponentDeleted;
    bool OnComponentDeleted(const Events::ComponentDeleted& e);
    EventRelay<Server, Events::PlayerDamage> m_EPlayerDamage;
    bool OnPlayerDamage(const Events::PlayerDamage& e);
    EventRelay<Server, Events::AmmoPickup> m_EAmmoPickup;
    bool OnAmmoPickup(const Events::AmmoPickup& e);
    EventRelay<Server, Events::PlayerDeath> m_EPlayerDeath;
    bool OnPlayerDeath(const Events::PlayerDeath& e);
    EventRelay<Server, Events::Win> m_EWin;
    bool OnWin(const Events::Win& e);
};

#endif

