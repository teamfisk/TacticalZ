#ifndef Client_h__
#define Client_h__

#include <string>
#include <ctime>

#include <glm/common.hpp>
#include <GLFW/glfw3.h> // For input event

#include "Network/MessageType.h"
#include "Network/NetworkDefines.h"
#include "Network/PlayerDefinition.h"
#include "Network/WinLeakCheck.h"
#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Core/EKeyDown.h"


class Client
{
public:
	Client();
	~Client();
	void Start(World* world, EventBroker* eventBroker);
    void Close();

private:
	// Threaded
	void ReadFromServer();

	int Receive(char* data, size_t length);
	int CreateMessage(MessageType type, std::string message, char* data);
    void Connect();
    void Disconnect();
    void Ping();
	void MoveMessageHead(char*& data, size_t& length, size_t stepSize);
	void ParseMessageType(char* data, size_t length);
	void ParseEventMessage(char* data, size_t length);
	void ParseConnect(char* data, size_t length);
	void ParsePing();
	void ParseServerPing();
	void ParseSnapshot(char* data, size_t length);
	void CreateNewPlayer(int i);

	// udp stuff
	boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
	boost::asio::io_service m_IOService;
	boost::asio::ip::udp::socket m_Socket;

	World* m_World;
	int m_PlayerID = -1;
	glm::vec2 m_PlayerPositions[MAXCONNECTIONS];
	//std::string m_PlayerNames[MAXCONNECTIONS];
	PlayerDefinition m_PlayerDefinitions[MAXCONNECTIONS];
	std::clock_t m_StartPingTime;
	double m_DurationOfPingTime;
	std::string m_PlayerName;
    bool m_ThreadIsRunning = true;

	// Events
	EventBroker* m_EventBroker;
	EventRelay<Client, Events::KeyDown> m_EKeyDown;
	bool OnKeyDown(const Events::KeyDown &e);
};

#endif

