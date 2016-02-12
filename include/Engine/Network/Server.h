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
    Server(World* world, EventBroker* eventBroker, int port);
    ~Server();

    void Update() override;

private:
    int m_Port = 27666;
    // UDP logic
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::io_service m_IOService;
    std::unique_ptr<boost::asio::ip::udp::socket> m_Socket;

    // Sending messages to client logic
    std::map<PlayerID, PlayerDefinition> m_ConnectedPlayers;
    // HACK: Fix INPUTSIZE
    char readBuffer[INPUTSIZE] = { 0 };
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
    std::vector<Events::InputCommand> m_InputCommandsToBroadcast;

    //Timers
    std::clock_t m_StartPingTime;
    
    // Packet loss logic
    PacketID m_PacketID = 0;
    PacketID m_PreviousPacketID = 0;

    // Private member functions
    size_t receive(char* data);
    void readFromClients();
    void send(PlayerID player, Packet& packet);
    void send(Packet& packet);
    void broadcast(Packet& packet);
    void sendSnapshot();
    void addChildrenToPacket(Packet& packet, EntityID entityID);
    void addInputCommandsToPacket(Packet& packet);
    void sendPing();
    void checkForTimeOuts();
    void disconnect(PlayerID playerID);
    void parseMessageType(Packet& packet);
    void parseOnInputCommand(Packet& packet);
    void parseOnPlayerDamage(Packet& packet);
    void parseConnect(Packet& packet);
    void parseDisconnect();
    void parseClientPing();
    void parsePing();
    void identifyPacketLoss();
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
    bool shouldSendToClient(EntityWrapper childEntity);
};

#endif

