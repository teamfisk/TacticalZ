#include "Network/TCPServer.h"
using namespace boost::asio::ip;

TCPServer::TCPServer()
{
    acceptor = std::unique_ptr<tcp::acceptor>(new tcp::acceptor(m_IOService, tcp::endpoint(tcp::v4(), 27666)));
}

TCPServer::~TCPServer()
{

}

void TCPServer::Start(World* world, EventBroker* eventBroker)
{
    Server::Start(world, eventBroker);
}

void TCPServer::readFromClients()
{
    acceptNewConnections();
    for (auto& kv : m_ConnectedPlayers) {
        while (kv.second.TCPSocket->available()) {
            try {
                bytesRead = receive(readBuffer, *kv.second.TCPSocket);
                lastReceivedSocket = kv.second.TCPSocket;
                Packet packet(readBuffer, bytesRead);
                parseMessageType(packet);
            } catch (const std::exception& err) {
                //LOG_ERROR("%i: Read from client crashed %s", m_PacketID, err.what());
            }
        }
    }

    std::clock_t currentTime = std::clock();
    // Send snapshot
    if (snapshotInterval < (1000 * (currentTime - previousSnapshotMessage) / (double)CLOCKS_PER_SEC)) {
        sendSnapshot();
        previousSnapshotMessage = currentTime;
    }

    // Send pings each 
    if (pingIntervalMs < (1000 * (currentTime - previousePingMessage) / (double)CLOCKS_PER_SEC)) {
        sendPing();
        previousePingMessage = currentTime;
    }

    // Time out logic
    if (checkTimeOutInterval < (1000 * (currentTime - timOutTimer) / (double)CLOCKS_PER_SEC)) {
        checkForTimeOuts();
        timOutTimer = currentTime;
    }
}

void TCPServer::acceptNewConnections()
{
    boost::shared_ptr<tcp::socket> newSocket = boost::shared_ptr<tcp::socket>(new tcp::socket(m_IOService));
    m_IOService.poll();
    acceptor->async_accept(*newSocket,
        boost::bind(&TCPServer::handle_accept, this, newSocket,
            boost::asio::placeholders::error));
}

void TCPServer::handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error)
{
    if (!error) {
        // Add tcp socket to connections
        PlayerDefinition pd;
        pd.StopTime = std::clock();
        pd.TCPSocket = socket;
        m_ConnectedPlayers[m_NextPlayerID++] = pd;
    }
}
void TCPServer::parseClientPing()
{

}
void TCPServer::parsePing()
{

}
void TCPServer::parseDisconnect()
{

}
void TCPServer::parseConnect(Packet & packet)
{

}
void TCPServer::parseOnInputCommand(Packet & packet)
{

}
void TCPServer::parsePlayerTransform(Packet & packet)
{

}
void TCPServer::send(Packet & packet, PlayerDefinition & playerDefinition)
{
    try {
        int bytesSent = playerDefinition.TCPSocket->send(
            boost::asio::buffer(packet.Data(), packet.Size()),
            0);
        // Network Debug data
        if (isReadingData) {
            m_NetworkData.TotalDataSent += packet.Size();
            m_NetworkData.DataSentThisInterval += packet.Size();
            m_NetworkData.AmountOfMessagesSent++;
        }
    } catch (const boost::system::system_error& e) {
        // TODO: Clean up invalid endpoints out of m_ConnectedPlayers later
        playerDefinition.Endpoint = boost::asio::ip::udp::endpoint();
    }
}

void TCPServer::send(Packet & packet)
{
    lastReceivedSocket->send(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        0);
    if (isReadingData) {
        // Network Debug data
        m_NetworkData.TotalDataSent += packet.Size();
        m_NetworkData.DataSentThisInterval += packet.Size();
    }
}

//boost::shared_ptr<boost::asio::ip::tcp::socket> socket
int TCPServer::receive(char * data,boost::asio::ip::tcp::socket& socket)
{
    unsigned int length = socket.read_some(
        boost::asio::buffer((void*)data, INPUTSIZE));
    // Network Debug data
    if (isReadingData) {
        m_NetworkData.TotalDataReceived += length;
        m_NetworkData.DataReceivedThisInterval += length;
        m_NetworkData.AmountOfMessagesReceived++;
    }
    return length;

}

PlayerID TCPServer::GetPlayerIDFromEndpoint(boost::asio::ip::udp::endpoint endpoint)
{
    return PlayerID();
}