#ifndef Server_h__
#define Server_h__

#include <string>
#include <ctime>

#include <glm/common.hpp>

#include "Network/MessageType.h"
#include "Network/NetworkDefinitions.h"
#include "Network/PlayerDefinition.h"
#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Network/Network.h"

class Server : public Network
{
public:
    Server();
    ~Server();
    void Start(World* m_world, EventBroker *eventBroker);
    void Update();
    void Close();

private:
    // UDP logic
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::socket m_Socket;
    PlayerDefinition m_PlayerDefinitions[MAXCONNECTIONS];

    //Timers
    std::clock_t m_StartPingTime;
    std::clock_t m_StopTimes[8];

    // Game logic
    World* m_World;
    EventBroker* m_EventBroker;
    // vec.size() = ammount of players to create, stores playerID's
    std::vector<unsigned int> m_PlayersToCreate;
    
    // Packet loss logic
    unsigned int m_PacketID;
    unsigned int m_PreviousPacketID;
    unsigned int m_SendPacketID;
    
    // Close logic
    bool m_ThreadIsRunning = true;

    // Private member functions
    int  receive(char* data, size_t length);
    void readFromClients();
    void send(Packet& packet, int playerID);
    void send(Packet& packet);
    void moveMessageHead(char*& data, size_t& length, size_t stepSize);
    void broadcast(std::string message);
    void broadcast(Packet& packet);
    void sendSnapshot();
    void sendPing();
    void checkForTimeOuts();
    void disconnect(int i);
    void parseMessageType(Packet& packet);
    void parseEvent(Packet& packet);
    void parseConnect(Packet& packet);
    void parseDisconnect();
    void parseClientPing();
    void parseServerPing();
    void parseSnapshot(Packet& packet);
    void identifyPacketLoss();
};

#endif

