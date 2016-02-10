#include "Network/TCPServer.h"
using namespace boost::asio::ip;

TCPServer::TCPServer()
{
    acceptor = std::unique_ptr<tcp::acceptor>(new tcp::acceptor(m_IOService, tcp::endpoint(tcp::v4(), 27666)));
}

TCPServer::~TCPServer()
{
}

void TCPServer::AcceptNewConnections(int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers)
{
    boost::shared_ptr<tcp::socket> newSocket = boost::shared_ptr<tcp::socket>(new tcp::socket(m_IOService));
    m_IOService.poll();
    acceptor->async_accept(*newSocket,
        boost::bind(&TCPServer::handle_accept, this, newSocket, boost::ref(nextPlayerID), boost::ref(connectedPlayers),
            boost::asio::placeholders::error));
}

PlayerID GetPlayerIDFromEndpoint(const std::map<PlayerID, PlayerDefinition>& connectedPlayers,
    boost::asio::ip::address address, unsigned short port)
{
    for (auto& kv : connectedPlayers) {
        if (kv.second.TCPAddress == address &&
            kv.second.TCPPort == port) {
            return kv.first;
        }
    }
    return -1;
}

void TCPServer::handle_accept(boost::shared_ptr<tcp::socket> socket, 
    int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers,
    const boost::system::error_code& error)
{
    if (!error && GetPlayerIDFromEndpoint(connectedPlayers, socket->remote_endpoint().address(),
        socket->remote_endpoint().port()) == -1) {
        // Add tcp socket to connections
        boost::asio::ip::tcp::no_delay option(true);
        socket->set_option(option);
        PlayerDefinition pd;
        pd.StopTime = std::clock();
        pd.TCPSocket = socket;
        pd.TCPAddress = socket.get()->remote_endpoint().address();
        pd.TCPPort = socket.get()->remote_endpoint().port();
        connectedPlayers[nextPlayerID++] = pd;
    }
}

void TCPServer::Send(Packet & packet, PlayerDefinition & playerDefinition)
{
    try {
        packet.UpdateSize();
        int bytesSent = playerDefinition.TCPSocket->send(
            boost::asio::buffer(packet.Data(), packet.Size()),
            0);
    } catch (const boost::system::system_error& e) {
        // TODO: Clean up invalid endpoints out of m_ConnectedPlayers later
        playerDefinition.Endpoint = boost::asio::ip::udp::endpoint();
    }
}

void TCPServer::Send(Packet & packet)
{
    packet.UpdateSize();
    lastReceivedSocket->send(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        0);
}

void TCPServer::Disconnect()
{ 

}

void TCPServer::Receive(Packet & packet, PlayerDefinition & playerDefinition)
{
    int bytesRead = readBuffer(m_ReadBuffer, playerDefinition);
    if (bytesRead > 0) {
        packet.ReconstructFromData(m_ReadBuffer, bytesRead);
    }
    lastReceivedSocket = playerDefinition.TCPSocket;
}

int TCPServer::readBuffer(char* data, PlayerDefinition & playerDefinition)
{
    if (!playerDefinition.TCPSocket) {
        return 0;
    }
    boost::system::error_code error;
    // Read size of packet
    int bytesReceived = playerDefinition.TCPSocket->read_some(boost
        ::asio::buffer((void*)data, sizeof(int)),
        error);
    int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, data, sizeof(int));

    // Read the rest of the message
    bytesReceived += playerDefinition.TCPSocket->read_some(boost
        ::asio::buffer((void*)(data + bytesReceived), sizeOfPacket - bytesReceived),
        error);
    if (error) {
        //LOG_ERROR("receive: %s", error.message().c_str());
    }
    return bytesReceived;
}