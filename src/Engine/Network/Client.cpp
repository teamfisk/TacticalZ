#include "Network/Client.h"

using namespace boost::asio::ip;


Client::Client() : m_Socket(m_IOService)
{
	// Set up network stream
	m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address::from_string("192.168.1.6"), 13);
	//Start(); // All logic happens here
}

Client::~Client()
{

}

void Client::Start()
{
	std::cout << "Please enter you name: ";
	std::cin >> m_PlayerName;
	while (m_PlayerName.size() > 7) {
		std::cout << "Please enter you name(No longer than 7 characters): ";
		std::cin >> m_PlayerName;
	}

	boost::thread_group threads;
	socket_ptr sock(new udp::socket(m_IOService));

	for (size_t i = 0; i < BOARDSIZE; i++) {
		for (size_t j = 0; j < BOARDSIZE; j++) {
			m_GameBoard[j][i] = ' ';
		}
	}

	for (size_t i = 0; i < MAXCONNECTIONS; i++) {
		m_PlayerPositions[i].x = -1;
		m_PlayerPositions[i].y = -1;
		m_PlayerNames[i] = "X";
	}

	m_Socket.connect(m_ReceiverEndpoint);
	std::cout << "I am client. BIP BOP\n";

	threads.create_thread(boost::bind(&Client::DisplayLoop, this));
	threads.create_thread(boost::bind(&Client::ReadFromServer, this));
	threads.create_thread(boost::bind(&Client::InputLoop, this));

	threads.join_all();
}

void Client::Close()
{
    Disconnect();
    m_ThreadIsRunning = false;
}

void Client::InputLoop()
{
	int intervallMs = 33; // ~30 times per second
	int commandInterval = 200; // for commands like ping and connect, name might be ambigiuos
	std::clock_t previousInputTime = std::clock();
	std::clock_t previousCommandTime = std::clock();

	while (m_ThreadIsRunning) {

		std::clock_t currentTime = std::clock();
		int testTimeShit = (1000 * (currentTime - previousCommandTime) / (double)CLOCKS_PER_SEC);
		if (commandInterval < (1000 * (currentTime - previousCommandTime) / (double)CLOCKS_PER_SEC)) {
			SendDebugInput();
			previousCommandTime = currentTime;
		}

		int testTimeShit2 = (1000 * (currentTime - previousInputTime) / (double)CLOCKS_PER_SEC);
		if (intervallMs < (1000 * (currentTime - previousInputTime) / (double)CLOCKS_PER_SEC)) {
			if (m_PlayerID != -1) {
				SendInput();
			}
			previousInputTime = currentTime;
		}
	}
}

void Client::DisplayLoop()
{
	while (m_ThreadIsRunning) {
		// Update gameboard
		for (size_t i = 0; i < BOARDSIZE; i++) {
			for (size_t j = 0; j < BOARDSIZE; j++) {
				m_GameBoard[j][i] = ' ';
			}
		}
		if (m_ShouldDrawGameBoard)
			DrawBoard();
		boost::this_thread::sleep(boost::posix_time::millisec(100));
	}
}

void Client::ReadFromServer()
{
	int bytesRead = -1;
	char readBuf[1024] = { 0 };

	while (m_ThreadIsRunning) {
		if (m_Socket.available()) {
			bytesRead = Receive(readBuf, INPUTSIZE);
			ParseMessageType(readBuf, bytesRead);
		}
	}
}

void Client::ParseMessageType(char* data, size_t length)
{
	int messageType = -1;
	memcpy(&messageType, data, sizeof(int)); // Read what type off message was sent from server
	MoveMessageHead(data, length, sizeof(int)); // Move the message head to know where to read from

	switch (static_cast<MessageType>(messageType)) {
	case MessageType::Connect:
		ParseConnect(data, length);
		break;
	case MessageType::ClientPing:
		ParsePing();
		break;
	case MessageType::ServerPing:
		ParseServerPing();
		break;
	case MessageType::Message:
		break;
	case MessageType::Snapshot:
		ParseSnapshot(data, length);
		break;
	case MessageType::Disconnect:
		break;
	case MessageType::Event:
		ParseEventMessage(data, length);
		break;
	default:
		break;
	}
}

void Client::ParseConnect(char* data, size_t len)
{
	memcpy(&m_PlayerID, data, sizeof(int));
	std::cout << "I am player: " << m_PlayerID << std::endl;
}

void Client::ParsePing()
{
	m_DurationOfPingTime = 1000 * (std::clock() - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
	std::cout << "response time with ctime(ms): " << m_DurationOfPingTime << std::endl;
}

void Client::ParseServerPing()
{
	char* testMsg = new char[128];
	int testOffset = CreateMessage(MessageType::ServerPing, "Ping recieved", testMsg);

	//std::cout << "Parsing ping." << std::endl;

	m_Socket.send_to(boost::asio::buffer(
		testMsg,
		testOffset),
		m_ReceiverEndpoint, 0);
}

void Client::ParseEventMessage(char* data, size_t length)
{
	int Id = -1;
	std::string command = std::string(data);
	if (command.find("+Player") != std::string::npos) {
		MoveMessageHead(data, length, command.size() + 1);
		memcpy(&Id, data, sizeof(int));
		MoveMessageHead(data, length, sizeof(int));
		// Sett Player name
		m_PlayerNames[Id] = command.erase(0, 7);
	}
	else {
		std::cout << "Event message: " << std::string(data) << std::endl;
	}

	MoveMessageHead(data, length, std::string(data).size() + 1);
}

void Client::ParseSnapshot(char* data, size_t length)
{
	std::string tempName;
	for (size_t i = 0; i < MAXCONNECTIONS; i++) {
		memcpy(&m_PlayerPositions[i].x, data, sizeof(float));
		MoveMessageHead(data, length, sizeof(float));
		memcpy(&m_PlayerPositions[i].y, data, sizeof(float));
		MoveMessageHead(data, length, sizeof(float));
		tempName = std::string(data);
		m_PlayerNames[i] = tempName;
		// +1 for null terminator
		MoveMessageHead(data, length, tempName.size() + 1);
	}
}

void Client::DrawBoard()
{
	for (size_t i = 0; i < MAXCONNECTIONS; i++) {
		if (m_PlayerPositions[i].x != -1 && m_PlayerPositions[i].y != -1) {
			m_GameBoard[static_cast<int>(m_PlayerPositions[i].x)][static_cast<int>(m_PlayerPositions[i].y)] = m_PlayerNames[i][0];
		}
	}

	system("cls");
	for (size_t i = 0; i < BOARDSIZE; i++) {
		std::cout << '_';
	}

	std::cout << std::endl;
	for (size_t i = 0; i < BOARDSIZE; i++) {
		for (size_t j = 0; j < BOARDSIZE; j++) {
			std::cout << m_GameBoard[j][i];
		}
		std::cout << std::endl;
	}

	for (size_t i = 0; i < BOARDSIZE; i++) {
		std::cout << "^";
	}
}

int Client::Receive(char* data, size_t length)
{
	int bytesReceived = m_Socket.receive_from(boost
		::asio::buffer((void*)data, length),
		m_ReceiverEndpoint,
		0);
	return bytesReceived;
}

int Client::CreateMessage(MessageType type, std::string message, char* data)
{
	int lengthOfMessage = 0;
	int messageType = static_cast<int>(type);
	lengthOfMessage = message.size();

	int offset = 0;
	// Message type
	memcpy(data + offset, &messageType, sizeof(int));
	offset += sizeof(int);
	// Message, add one extra byte for null terminator
	memcpy(data + offset, message.data(), (lengthOfMessage + 1) * sizeof(char));
	offset += (lengthOfMessage + 1) * sizeof(char);

	return offset;
}

void Client::Disconnect()
{
    char* dataPackage = new char[INPUTSIZE]; // The package that will be sent to the server, when filled
    int len = CreateMessage(MessageType::Disconnect, "+Disconnect", dataPackage);
    m_Socket.send_to(boost::asio::buffer(
        dataPackage,
        len),
        m_ReceiverEndpoint, 0);
}

void Client::MoveMessageHead(char*& data, size_t& length, size_t stepSize)
{
	data += stepSize;
	length -= stepSize;
}

void Client::SendDebugInput()
{
	char* dataPackage = new char[INPUTSIZE];
	if (GetAsyncKeyState('P')) { // Maybe use previous key here
		int length = CreateMessage(MessageType::ClientPing, "Ping", dataPackage);
		m_StartPingTime = std::clock();
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			length),
			m_ReceiverEndpoint, 0);
	}

	if (GetAsyncKeyState('C')) {
		int length = CreateMessage(MessageType::Connect, m_PlayerName, dataPackage);
		m_StartPingTime = std::clock();
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			length),
			m_ReceiverEndpoint, 0);
	}

	if (GetAsyncKeyState('Q')) { // Does not work. Plez fix
		exit(1);
	}
	memset(dataPackage, 0, INPUTSIZE);
	delete[] dataPackage;
}

void Client::SendInput()
{
	char* dataPackage = new char[INPUTSIZE]; // The package that will be sent to the server, when filled
	if (GetAsyncKeyState('W')) {
		int len = CreateMessage(MessageType::Event, "+Forward", dataPackage);
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			len),
			m_ReceiverEndpoint, 0);
	}
	if (GetAsyncKeyState('A')) {
		int len = CreateMessage(MessageType::Event, "-Right", dataPackage);
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			len),
			m_ReceiverEndpoint, 0);
	}
	if (GetAsyncKeyState('S')) {
		int len = CreateMessage(MessageType::Event, "-Forward", dataPackage);
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			len),
			m_ReceiverEndpoint, 0);
	}
	if (GetAsyncKeyState('D')) {
		int len = CreateMessage(MessageType::Event, "+Right", dataPackage);
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			len),
			m_ReceiverEndpoint, 0);
	}
	if (GetAsyncKeyState('V')) {
        Disconnect();
	}

	memset(dataPackage, 0, INPUTSIZE);
	delete[] dataPackage;
}