#include "Network/UDPServer.h"

UDPServer::UDPServer()
{
    m_Socket = std::unique_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 27666)));
}

UDPServer::UDPServer(int port)
{
    m_Socket = std::unique_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(m_IOService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)));
}

UDPServer::~UDPServer()
{ }
// TODO: Fix correct groups
void UDPServer::Send(Packet& packet, PlayerDefinition& playerDefinition)
{
    packet.UpdateSize();
    try {
        //Debug
        int debugTheSixeOfpacket = packet.Size();
        //Debug end
        // Remove header from packet.
        packet.ReadData(packet.HeaderSize());
        int totalBytesSent = 0;
        int groupIndex = 1;
        int groupSize = std::ceil((float)packet.Size() / MAXPACKETSIZE);
        int packetDataSent = 0;
        int packetDataSize = packet.Size() - packet.HeaderSize();

        while (packetDataSize > packetDataSent) {
            Packet splitPacket(packet.GetMessageType(), playerDefinition.PacketID);
            splitPacket.ChangeGroupIndex(groupIndex);
            splitPacket.ChangeGroupSize(groupSize);
            splitPacket.ChangeGroup(playerDefinition.PacketGroup);
            int amountToSend = packetDataSize - packetDataSent;
            if (amountToSend > MAXPACKETSIZE) {
                amountToSend = MAXPACKETSIZE;
            }
            splitPacket.WriteData(packet.ReadData(amountToSend), amountToSend);
            splitPacket.UpdateSize();
            // Remove header size from bytes sent soo that we only
            // count data in the packet
            int bytesSent = 0;
            bytesSent = m_Socket->send_to(
                boost::asio::buffer(splitPacket.Data(), splitPacket.Size()),
                playerDefinition.Endpoint,
                0);
            packetDataSent += bytesSent - splitPacket.HeaderSize();
            totalBytesSent += bytesSent;
            //LOG_INFO("bytesSent size %i, groupIndex: %i. Number of packets: %i", bytesSent, sequenceNumber, totalMessages);
            ++groupIndex;
        }
        playerDefinition.PacketGroup++;
    } catch (const boost::system::system_error& e) {
        LOG_INFO(e.what());
        // TODO: Clean up invalid endpoints out of m_ConnectedPlayers later
        playerDefinition.Endpoint = boost::asio::ip::udp::endpoint();
    }
}

void UDPServer::SendToConnectedPlayers(Packet& packet, std::map<PlayerID, PlayerDefinition>& playersTosendTo)
{
    // Work in progress
    packet.UpdateSize();

    //Debug
    int debugTheSixeOfpacket = packet.Size();
    //Debug end
    // Remove header from packet.
    packet.ReadData(packet.HeaderSize());
    int totalBytesSent = 0;
    int groupIndex = 1;
    int groupSize = std::ceil((float)packet.Size() / MAXPACKETSIZE);
    int packetDataSent = 0;
    int packetDataSize = packet.Size() - packet.HeaderSize();

    while (packetDataSize > packetDataSent) {
        Packet splitPacket(packet.GetMessageType());
        splitPacket.ChangeGroupIndex(groupIndex);
        splitPacket.ChangeGroupSize(groupSize);
        int amountToSend = packetDataSize - packetDataSent;
        if (amountToSend > MAXPACKETSIZE) {
            amountToSend = MAXPACKETSIZE;
        }
        splitPacket.WriteData(packet.ReadData(amountToSend), amountToSend);
        splitPacket.UpdateSize();
        // Remove header size from bytes sent soo that we only
        // count data in the packet
        int bytesSent = 0;
        for (auto& kv : playersTosendTo) {
            try {
                splitPacket.ChangeGroup(kv.second.PacketGroup);
                bytesSent = m_Socket->send_to(
                    boost::asio::buffer(splitPacket.Data(), splitPacket.Size()),
                    kv.second.Endpoint,
                    0);
                LOG_INFO("bytesSent: %i", bytesSent);
            } catch (const boost::system::system_error& e) {
                LOG_INFO(e.what());
                // TODO: Clean up invalid endpoints out of m_ConnectedPlayers later
                kv.second.Endpoint = boost::asio::ip::udp::endpoint();
            }
        }
            packetDataSent += splitPacket.Size() - splitPacket.HeaderSize();
            totalBytesSent += splitPacket.Size();

            //LOG_INFO("bytesSent size %i, groupIndex: %i. Number of packets: %i", bytesSent, sequenceNumber, totalMessages);
            ++groupIndex;
    }
    for (auto& kv : playersTosendTo) {
        kv.second.PacketGroup++;
    }
}

// Send back to endpoint of received packet
void UDPServer::Send(Packet & packet)
{
    packet.UpdateSize();
    size_t bytesSent = m_Socket->send_to(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        m_ReceiverEndpoint,
        0);
    LOG_INFO("Size of packet is %i", bytesSent);
}

// Broadcasting respond specific logic
void UDPServer::Send(Packet & packet, boost::asio::ip::udp::endpoint endpoint)
{
    packet.UpdateSize();
    size_t bytesSent = m_Socket->send_to(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        endpoint,
        0);
    LOG_INFO("Size of packet is %i", bytesSent);
}

// Broadcasting
void UDPServer::Broadcast(Packet & packet, int port)
{
    packet.UpdateSize();
    m_Socket->set_option(boost::asio::socket_base::broadcast(true));
    size_t bytesSent = m_Socket->send_to(
        boost::asio::buffer(
            packet.Data(),
            packet.Size()),
        boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4().broadcast(), port),
        0);
    m_Socket->set_option(boost::asio::socket_base::broadcast(false));
}

void UDPServer::Receive(Packet & packet, PlayerDefinition & playerDefinition)
{
    int bytesRead = readBuffer();
    if (bytesRead > 0) {
        packet.ReconstructFromData(m_ReadBuffer, bytesRead);
    }
    playerDefinition.Endpoint = m_ReceiverEndpoint;
}

bool UDPServer::IsSocketAvailable()
{
    return m_Socket->available();
}

int UDPServer::readBuffer()
{
    if (!m_Socket) {
        return 0;
    }
    int addasdasd = m_Socket->available();
    boost::system::error_code error;
    // Read size of packet
    m_Socket->receive_from(boost
        ::asio::buffer((void*)m_ReadBuffer, sizeof(int)),
        m_ReceiverEndpoint, boost::asio::ip::udp::socket::message_peek, error);
    unsigned int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, m_ReadBuffer, sizeof(int));

    if (sizeOfPacket > m_Socket->available()) {
        LOG_WARNING("UDPServer::readBuffer(): We haven't got the whole packet yet.");
        //return 0;
    }

    // if the buffer is to small increase the size of it
    if (sizeOfPacket > m_BufferSize) {
        delete[] m_ReadBuffer;
        m_ReadBuffer = new char[sizeOfPacket];
        m_BufferSize = sizeOfPacket;
    }

    // Read the rest of the message
    size_t bytesReceived = m_Socket->receive_from(boost
        ::asio::buffer((void*)(m_ReadBuffer),
            sizeOfPacket),
        m_ReceiverEndpoint, 0, error);
    if (error) {
        //LOG_ERROR("receive: %s", error.message().c_str());
    }
    if (sizeOfPacket > 1000000)
        LOG_WARNING("The packets received are bigger than 1MB");

    return bytesReceived;
}

void UDPServer::AcceptNewConnections(int& nextPlayerID, std::map<PlayerID, PlayerDefinition>& connectedPlayers)
{ }