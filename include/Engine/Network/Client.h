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
#include "Network/UDPServer.h" //LOL
#include "Network/TCPClient.h"
#include "Network/SnapshotDefinitions.h"
#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Core/ConfigFile.h"
#include "Input/EInputCommand.h"
#include "Core/EPlayerDamage.h"
#include "Network/EInterpolate.h"
#include "Network/SnapshotFilter.h"
#include "Core/EPlayerSpawned.h"

class Client : public Network
{
public:
    Client(World* world, EventBroker* eventBroker);
    Client(World* world, EventBroker* eventBroker, std::unique_ptr<SnapshotFilter> snapshotFilter);
    ~Client();

    void Connect(std::string address, int port);
    void Update() override;

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
    void parseHeartbeat(Packet& packet, PlayerDefinition);
    void parseKick();
    void parsePlayersSpawned(Packet& packet);
    void parseEntityDeletion(Packet& packet);
    void parseComponentDeletion(Packet& packet);
    void InterpolateFields(Packet & packet, const ComponentInfo & componentInfo, const EntityID & entityID, const std::string & componentType);
    void parseSnapshot(Packet& packet);
    void identifyPacketLoss();
    void hasServerTimedOut();
    EntityID createPlayer();
    void sendInputCommands();
    void sendLocalPlayerTransform();
    void becomePlayer();
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
    void parsePlayerDamage(Packet& packet);
private:
    UDPClient m_Unreliable;
    UDPServer m_Heartbeat;
    TCPClient m_Reliable;
};

#endif

