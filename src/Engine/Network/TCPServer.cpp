#include "Network/TCPServer.h"
using namespace boost::asio::ip;

TCPServer::TCPServer() 
{
    acceptor = std::unique_ptr<tcp::acceptor>(new tcp::acceptor(m_IOService, tcp::endpoint(tcp::v4(), 27666)));
    // Make the acceptor non-blocking so we wont get stuck in AcceptNewConnections().
    acceptor->non_blocking(true);
    m_Port = GetPort();
    m_Address = GetAddress();
}

TCPServer::~TCPServer()
{ }

void TCPServer::AcceptNewConnections(int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers)
{
    boost::system::error_code error;
    boost::shared_ptr<tcp::socket> newSocket = boost::shared_ptr<tcp::socket>(new tcp::socket(m_IOService));
    acceptor->accept(*newSocket, error);
    // If no error occured add new tcp connection
    if (!error) {
        // Add tcp socket to connections
        boost::asio::ip::tcp::no_delay option(true);
        newSocket->set_option(option);
        PlayerDefinition pd;
        pd.StopTime = std::clock();
        pd.TCPSocket = newSocket;
        pd.TCPAddress = newSocket.get()->remote_endpoint().address();
        pd.TCPPort = newSocket.get()->remote_endpoint().port();
        connectedPlayers[nextPlayerID++] = pd;
    }
}

PlayerID TCPServer::getPlayerIDFromEndpoint(const std::map<PlayerID, PlayerDefinition>& connectedPlayers,
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

void TCPServer::Send(Packet & packet, PlayerDefinition & playerDefinition)
{
    packet.UpdateSize();
    try {
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

int TCPServer::GetPort()
{
    return acceptor->local_endpoint().port();
}

std::string TCPServer::GetAddress()
{
    boost::asio::ip::tcp::resolver resolver(m_IOService);
    boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), boost::asio::ip::host_name(), "");
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
    boost::asio::ip::tcp::endpoint endpoint = *it;
    return endpoint.address().to_string().c_str();
}

void TCPServer::Receive(Packet & packet, PlayerDefinition & playerDefinition)
{
    int bytesRead = readBuffer(playerDefinition);
    if (bytesRead > 0) {
        packet.ReconstructFromData(m_ReadBuffer, bytesRead);
    }
    lastReceivedSocket = playerDefinition.TCPSocket;
}

int TCPServer::readBuffer(PlayerDefinition & playerDefinition)
{
    if (!playerDefinition.TCPSocket) {
        return 0;
    }
    boost::system::error_code error;
    // Read size of packet
    playerDefinition.TCPSocket->receive(boost
        ::asio::buffer((void*)m_ReadBuffer, sizeof(int)),
        boost::asio::ip::tcp::socket::message_peek, error);
    unsigned int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, m_ReadBuffer, sizeof(int));

    // if the buffer is to small increase the size of it
    if (sizeOfPacket > m_BufferSize) {
        delete[] m_ReadBuffer;
        m_ReadBuffer = new char[sizeOfPacket];
        m_BufferSize = sizeOfPacket;
    }
    // Read the rest of the message
    size_t bytesReceived = playerDefinition.TCPSocket->read_some(boost
        ::asio::buffer((void*)(m_ReadBuffer), sizeOfPacket),
        error);
    if (error) {
        //LOG_ERROR("receive: %s", error.message().c_str());
    }
    if (sizeOfPacket > 1000000)
        LOG_WARNING("The packets received are bigger than 1MB");

    return bytesReceived;
}