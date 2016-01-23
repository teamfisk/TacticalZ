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
#include "Network/SnapshotDefinitions.h"
#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Core/ConfigFile.h"
#include "Input/EInputCommand.h"
#include "Core/EPlayerDamage.h"
#include "Network/EInterpolate.h"
#include "Game/Events/EPlayerSpawned.h"

class Client : public Network
{
public:
    Client(ConfigFile* config);
    ~Client();
    void Start(World* world, EventBroker* eventBroker) override;
    void Update() override;
private:
    // Assio UDP logic
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::socket m_Socket;

    // Sending message to server logic
    int bytesRead = -1;
    char readBuf[INPUTSIZE] = { 0 };

    // Packet loss logic
    PacketID m_PacketID = 0;
    PacketID m_PreviousPacketID = 0;
    PacketID m_SendPacketID = 0;

    // Game logic
    World* m_World;
    std::string m_PlayerName;
    PlayerID m_PlayerID = -1;
    EntityID m_ServerEntityID = std::numeric_limits<EntityID>::max();
    bool m_IsConnected = false;
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
    void readFromServer();
    int  receive(char* data);
    void send(Packet& packet);
    void connect();
    void disconnect();
    void parseMessageType(Packet& packet);
    void updateFields(Packet& packet, const ComponentInfo& componentInfo, const EntityID& entityID, const std::string& componentType);
    void parseConnect(Packet& packet);
    void parsePlayerConnected(Packet& packet);
    void parsePing();
    void parseKick();
    void parsePlayersSpawned(Packet& packet);
    void InterpolateFields(Packet & packet, const ComponentInfo & componentInfo, const EntityID & entityID, const std::string & componentType);
    void parseSnapshot(Packet& packet);
    void identifyPacketLoss();
    bool hasServerTimedOut();
    EntityID createPlayer();
    void sendInputCommands();
    void becomePlayer();
    // Mapping Logic
    // Returns if local EntityID exist in map
    bool clientServerMapsHasEntity(EntityID clientEntityID);
    // Returns if server EntityID exist in map
    bool serverClientMapsHasEntity(EntityID serverEntityID);
    void insertIntoServerClientMaps(EntityID serverEntityID, EntityID clientEntityID);

    // Events
    EventBroker* m_EventBroker;
    EventRelay<Client, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<Client, Events::PlayerDamage> m_EPlayeDamage;
    bool OnPlayerDamage(const Events::PlayerDamage& e);
};

#endif

