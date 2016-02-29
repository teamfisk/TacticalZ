#ifndef Client_h__
#define Client_h__

#include <string>
#include <ctime>
#include <limits>
#include <queue>

#include <glm/common.hpp>
#include <boost/asio.hpp>
#include <boost/shared_array.hpp>

#include "Network/Network.h"
#include "Network/MessageType.h"
#include "Network/PlayerDefinition.h"
#include "Network/UDPClient.h"
#include "Network/TCPClient.h"
#include "Network/SnapshotDefinitions.h"
#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Core/ConfigFile.h"
#include "Core/EPlayerDeath.h"
#include "Input/EInputCommand.h"
#include "Core/EPlayerDamage.h"
#include "../Game/Events/EDoubleJump.h"
#include "Network/EInterpolate.h"
#include "Network/SnapshotFilter.h"
#include "Core/EPlayerSpawned.h"
#include "Network/ESearchForServers.h"

struct ServerInfo
{
    ServerInfo(std::string a, int b, std::string c, int d)
    {
        Address = a; Port = b; Name = c; PlayersConnected = d;
    }
    std::string Address = "";
    int Port = 0;
    std::string Name = "";
    int PlayersConnected = 0;
};

class Client : public Network
{
public:
    Client(World* world, EventBroker* eventBroker);
    Client(World* world, EventBroker* eventBroker, std::unique_ptr<SnapshotFilter> snapshotFilter);
    ~Client();

    void Connect(std::string address, int port);
    void Update() override;
private:
    UDPClient m_Unreliable;
    TCPClient m_Reliable;
    std::vector<Events::PlayerSpawned> m_PlayerSpawnEvents;
    void parseSpawnEvents();
    // Save for children
    std::unique_ptr<SnapshotFilter> m_SnapshotFilter = nullptr;

    std::string m_Address;
    int m_Port = 0;
    // Sending message to server logic
    size_t bytesRead = 0;

    // Packet loss logic
    PacketID m_PacketID = 0;
    PacketID m_PreviousPacketID = 0;
    PacketID m_SendPacketID = 0;

    // Game logic
    std::string m_PlayerName;
    PlayerID m_PlayerID = -1;
    bool m_IsConnected = false;
    EntityWrapper m_LocalPlayer = EntityWrapper::Invalid;
    // Server Client Lookup map
    // Assumes that root node for client and server is EntityID 0.

    // Don't Add items to these two maps with insert, use insertIntoServerClientMaps(EntityID, EntityID)!!!!
    std::unordered_map<EntityID, EntityID> m_ServerIDToClientID;
    std::unordered_map<EntityID, EntityID> m_ClientIDToServerID;

    // Network logic
    PlayerDefinition m_PlayerDefinitions[8];
    SnapshotDefinitions m_NextSnapshot;
    double m_DurationOfPingTime;
    std::clock_t m_StartPingTime;
    std::clock_t m_TimeSinceSentInputs;
    unsigned int m_SendInputIntervalMs;
    std::vector<Events::InputCommand> m_InputCommandBuffer;

    // Private member functions
    size_t  receive(char* data);
    void disconnect();
    void parseMessageType(Packet& packet);
    void updateFields(Packet& packet, const ComponentInfo& componentInfo, const EntityID& entityID);
    SharedComponentWrapper createSharedComponent(Packet& packet, EntityID entityID, const ComponentInfo& componentInfo);
    void ignoreFields(Packet& packet, const ComponentInfo& componentInfo);
    void parseUDPConnect(Packet& packet);
    void parseTCPConnect(Packet& packet);
    void parsePlayerConnected(Packet& packet);
    void parsePing();
    void parseServerlist(Packet& packet);
    void parseKick();
    void parsePlayersSpawned(Packet& packet);
    void parseEntityDeletion(Packet& packet);
    void parsePlayerDamage(Packet& packet);
    void parseComponentDeletion(Packet& packet);
    void parseDoubleJump(Packet& packet);
    void InterpolateFields(Packet & packet, const ComponentInfo & componentInfo, const EntityID & entityID, const std::string & componentType);
    void parseSnapshot(Packet& packet);
    void identifyPacketLoss();
    void hasServerTimedOut();
    EntityID createPlayer();
    void sendInputCommands();
    void sendLocalPlayerTransform();
    void becomePlayer();
    void displayServerlist();
    // Mapping Logic
    // Returns if local EntityID exist in map
    bool clientServerMapsHasEntity(EntityID clientEntityID);
    // Returns if server EntityID exist in map
    bool serverClientMapsHasEntity(EntityID serverEntityID);
    void insertIntoServerClientMaps(EntityID serverEntityID, EntityID clientEntityID);
    void deleteFromServerClientMaps(EntityID serverEntityID, EntityID clientEntityID);

    // Events
    EventRelay<Client, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<Client, Events::PlayerDamage> m_EPlayerDamage;
    bool OnPlayerDamage(const Events::PlayerDamage& e);
    EventRelay<Client, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(const Events::PlayerSpawned& e);
    EventRelay< Client, Events::SearchForServers> m_ESearchForServers;
    EventRelay<Client, Events::DoubleJump> m_EDoubleJump;
    bool OnDoubleJump(Events::DoubleJump & e);
    bool OnSearchForServers(const Events::SearchForServers& e);
    UDPClient m_ServerlistRequest;
    std::vector<ServerInfo> m_Serverlist;
    bool m_SearchingForServers = false;
    std::clock_t m_StartSearchTime;
    double m_SearchingTime = 2000; // Config I guess
};

#endif

