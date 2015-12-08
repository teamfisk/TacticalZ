#ifndef Server_h__
#define Server_h__
#include <string>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <glm/common.hpp>
#include "NetworkDefines.h"
#include "MessageType.h"
#include "Core/World.h"
#include "Network/PlayerDefinition.h"

class Server
{
public:
    Server();
    ~Server();
    void Start(World* m_world);

private:
    // udp stuff
    boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
    boost::asio::io_service m_IOService;
    boost::asio::ip::udp::socket m_Socket;
  //  boost::asio::ip::udp::endpoint m_Connections[MAXCONNECTIONS];
    PlayerDefinition m_PlayerDefinitions[MAXCONNECTIONS];
    //Timers
    std::clock_t m_StartPingTime;
    std::clock_t m_StopTimes[8];
    // Game logic
  //  std::string m_PlayerNames[MAXCONNECTIONS];
    char m_GameBoard[BOARDSIZE][BOARDSIZE];
    glm::vec2 m_PlayerPositions[8];
    World* m_World;

    // Threaded
    void DisplayLoop();
    void ReadFromClients();
    void InputLoop();


    int  Receive(char* data, size_t length);
    int  CreateMessage(MessageType type, std::string message, char * data);
    void MoveMessageHead(char*& data, size_t& length, size_t stepSize);
    void Broadcast(std::string message);
    void Broadcast(char* data, size_t length);
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

