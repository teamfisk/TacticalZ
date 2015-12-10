#ifndef Server_h__
#define Server_h__

#include <string>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <glm/common.hpp>

#include "Network/MessageType.h"
#include "Network/NetworkDefinitions.h"
#include "Network/PlayerDefinition.h"
#include "Core/World.h"

class Server
{
public:
    Server();
    ~Server();
    void Start(World* m_world);
    void Close();

private:
    // udp stuff
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::socket m_Socket;
    PlayerDefinition m_PlayerDefinitions[MAXCONNECTIONS];
    //Timers
    std::clock_t m_StartPingTime;
    std::clock_t m_StopTimes[8];
    // Game logic
    World* m_World;
	// Packet loss logic
	unsigned int m_PacketCounter = 0;
	unsigned int m_PacketID = 0;
	const unsigned int m_PacketModolus = 1000;

    // Close logic
    bool m_ThreadIsRunning = true;
    // Threaded
    void DisplayLoop();
    void ReadFromClients();
    void InputLoop();



    int  Receive(char* data, size_t length);
	void Send(Package& package, int playerID);
	void Send(Package& package);
    int  CreateMessage(MessageType type, std::string message, char * data);
    void MoveMessageHead(char*& data, size_t& length, size_t stepSize);
    void Broadcast(std::string message);
    void Broadcast(Package& package);
    void SendSnapshot();
    void SendPing();
    void CheckForTimeOuts();
    int  CreateHeader(MessageType type, char* data);
    void Disconnect(int i);
    void ParseMessageType(char* data, size_t length);
    void ParseEvent(char* data, size_t length);
    void ParseConnect(char* data, size_t length);
    void ParseDisconnect();
    void ParseClientPing();
    void ParseServerPing();
    void ParseSnapshot(char* data, size_t length);
};

#endif

