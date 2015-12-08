#ifndef Client_h__
#define Client_h__

#include <string>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <glm/common.hpp>

#include "Network/MessageType.h"
#include "Network/NetworkDefines.h"
#include "Network/WinLeakCheck.h"
#include "Core/EventBroker.h"
#include "Core/EKeyDown.h"


class Client
{
public:
	Client();
	~Client();
	void Start(EventBroker* eventBroker);
    void Close();

private:
	// Threaded
	void DisplayLoop();
	void ReadFromServer();
	void InputLoop();

	int Receive(char* data, size_t length);
	int CreateMessage(MessageType type, std::string message, char* data);
    void Connect();
    void Disconnect();
	void MoveMessageHead(char*& data, size_t& length, size_t stepSize);
	void ParseMessageType(char* data, size_t length);
	void ParseEventMessage(char* data, size_t length);
	void ParseConnect(char* data, size_t length);
	void ParsePing();
	void ParseServerPing();
	void ParseSnapshot(char* data, size_t length);
	void SendInput();
	void SendDebugInput();
	void DrawBoard();

	// udp stuff
	boost::asio::ip::udp::endpoint m_ReceiverEndpoint;
	boost::asio::io_service m_IOService;
	boost::asio::ip::udp::socket m_Socket;

	int m_PlayerID = -1;
	char m_GameBoard[BOARDSIZE][BOARDSIZE];
	glm::vec2 m_PlayerPositions[MAXCONNECTIONS];
	std::string m_PlayerNames[MAXCONNECTIONS];
	std::clock_t m_StartPingTime;
	double m_DurationOfPingTime;
	bool m_ShouldDrawGameBoard = true;
	std::string m_PlayerName;
    bool m_ThreadIsRunning = true;

	// Events
	EventBroker* m_EventBroker;
	EventRelay<Client, Events::KeyDown> m_EKeyDown;
	bool OnKeyDown(const Events::KeyDown &e);
};

#endif

