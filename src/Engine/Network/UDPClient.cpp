#include "Network/UDPClient.h"
#include "boost/asio/basic_datagram_socket.hpp"

using namespace boost::asio::ip;

UDPClient::UDPClient()
{ }

UDPClient::~UDPClient()
{ }

bool UDPClient::Connect(std::string playerName, std::string address, int port)
{
    if (m_Socket) {
        return false;
    }
    m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address().from_string(address), port);
    m_Socket = boost::shared_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(m_IOService));
    m_Socket->open(boost::asio::ip::udp::v4());
    boost::asio::socket_base::receive_buffer_size option(819200);
    m_Socket->set_option(option);
    return true;
}

void UDPClient::Disconnect()
{
    m_Socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    m_Socket->close();
    m_Socket = nullptr;

    m_LastReceivedSnapshotGroup = 0;
    m_PacketSegmentMap.clear();
    PacketID m_SendPacketID = 0;
}

void UDPClient::Receive(Packet& packet)
{
    int bytesRead = readBuffer();
    if (bytesRead > 0) {
        packet.ReconstructFromData(m_ReadBuffer, bytesRead);
    }
}

void UDPClient::ReceivePackets()
{
    readPartOfPacket();
}

int UDPClient::readBuffer()
{
    if (!m_Socket) {
        return 0;
    }
    boost::system::error_code error;
    // Read size of packet
    m_Socket->receive(boost
        ::asio::buffer((void*)m_ReadBuffer, sizeof(int)),
        boost::asio::ip::udp::socket::message_peek, error);
    int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, m_ReadBuffer, sizeof(int));
    if (sizeOfPacket > m_Socket->available()) {
        LOG_WARNING("UDPClient::readBuffer(): We haven't got the whole packet yet.");
        //return 0;
    }
    // if the buffer is to small increase the size of it
    if (sizeOfPacket > m_BufferSize) {
        delete[] m_ReadBuffer;
        m_ReadBuffer = new char[sizeOfPacket];
        m_BufferSize = sizeOfPacket;
    }

    size_t availableData = m_Socket->available();
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

void UDPClient::readPartOfPacket()
{
    if (!m_Socket) {
        return;
    }
    boost::system::error_code error;
    // Peek header
    m_Socket->receive(boost
        ::asio::buffer((void*)m_ReadBuffer, 5 * sizeof(int)),
        boost::asio::ip::udp::socket::message_peek, error);

    int sizeOfPacket = 0;
    memcpy(&sizeOfPacket, m_ReadBuffer, sizeof(int));
    if (sizeOfPacket == 0) {
        return;
    }
    int packetGroup = 0;
    memcpy(&packetGroup, m_ReadBuffer + sizeof(int), sizeof(int));
    int packetGroupIndex = 0;
    memcpy(&packetGroupIndex, m_ReadBuffer + 2 * sizeof(int), sizeof(int));
    int packetGroupSize = 0;
    memcpy(&packetGroupSize, m_ReadBuffer + 3 * sizeof(int), sizeof(int));
    //LOG_INFO("Packet group: %i. Group index: %i. Group size: %i. Packet size: %i.", packetGroup, packetGroupIndex, packetGroupSize, sizeOfPacket);
    if (sizeOfPacket > m_Socket->available()) {
        LOG_WARNING("UDPClient::readBuffer(): We haven't got the whole packet yet.");
        // return;
    }
    // if the buffer is to small increase the size of it
    boost::shared_ptr<char> packetData(new char[sizeOfPacket]);

    // Read the message
    size_t bytesReceived = m_Socket->receive_from(boost
        ::asio::buffer((void*)(packetData.get()),
            sizeOfPacket),
        m_ReceiverEndpoint, 0, error);
    if (error) {
        LOG_ERROR("UDPClient::readPartOfPacket: %s", error.message().c_str());
    }
    // Might want to do this earlier when i figure out a good way to 
    // remove data from network buffer.
    if (hasReceivedPacket(packetGroup, packetGroupIndex)) {
        return;
    }
    //std::map<unsigned int, std::vector<std::pair<unsigned int, boost::shared_ptr<char>>>> packetSegmentMap;
    // If group exists
    if (m_PacketSegmentMap.find(packetGroup) != m_PacketSegmentMap.end()) {
        m_PacketSegmentMap.at(packetGroup).push_back(std::make_pair(packetGroupIndex, std::move(packetData)));
    } else { // Create group and add element
        m_PacketSegmentMap[packetGroup].push_back(std::make_pair(packetGroupIndex, std::move(packetData)));
    }
    return;
}

bool UDPClient::hasReceivedPacket(int packetGroup, int groupIndex)
{
    if (m_PacketSegmentMap.find(packetGroup) != m_PacketSegmentMap.end()) {
        const std::vector<std::pair<int, boost::shared_ptr<char>>>& loopPacketGroup = m_PacketSegmentMap.at(packetGroup);
        for (size_t i = 0; i < loopPacketGroup.size(); i++) {
            if (loopPacketGroup.at(i).first == groupIndex) {
                return true;
            }
        }
    }
    return false;
}

void UDPClient::Send(Packet& packet)
{
    packet.UpdateSize();
    m_Socket->send_to(boost::asio::buffer(
        packet.Data(),
        packet.Size()),
        m_ReceiverEndpoint, 0);
}

void UDPClient::Broadcast(Packet& packet, int port)
{
    packet.UpdateSize();
    m_Socket->set_option(boost::asio::socket_base::broadcast(true));
    m_Socket->send_to(boost::asio::buffer(
        packet.Data(),
        packet.Size()),
        udp::endpoint(boost::asio::ip::address_v4().broadcast(), port)
        , 0);
    m_Socket->set_option(boost::asio::socket_base::broadcast(false));
}

bool UDPClient::IsSocketAvailable()
{
    if (!m_Socket) {
        return false;
    }
    return m_Socket->available();
}

bool UDPClient::GetNextPacket(Packet & packet)
{
    // A duplicate packet should not be present in the vector!
    // Soo we will assume that this is true and only look if size
    // of vector is correct.
    std::map<unsigned int, std::vector<std::pair<int, boost::shared_ptr<char>>>>::iterator it = m_PacketSegmentMap.begin();
    while (it != m_PacketSegmentMap.end()) {
        // pair(Group index, packetData)
        std::vector<std::pair<int, boost::shared_ptr<char>>>& currentVector = it->second;
        Packet headerInfoPacket(currentVector.at(0).second.get(), packet.HeaderSize());
        int groupSize = headerInfoPacket.GroupSize();
        //LOG_INFO("UDPClient::GetNextPacket: Packet group : %i.Group index : %i.Group size : %i. lastReceivedSnapshotGroup: %i. MessageType(Ples 4): %i", headerInfoPacket.Group(), headerInfoPacket.GroupIndex(), groupSize, lastReceivedSnapshotGroup, headerInfoPacket.GetMessageType());
        //LOG_INFO("UDPClient::GetNextPacket: Packet group : %i.lastReceivedSnapshotGroup: %i. MessageType(Ples 4): %i", headerInfoPacket.Group(), lastReceivedSnapshotGroup, headerInfoPacket.GetMessageType());
        int mapSize = m_PacketSegmentMap.size();
        if (mapSize > 5) {
            it = m_PacketSegmentMap.erase(it);
            LOG_INFO("The map is increasing in size, size is %i", mapSize);
            continue;
        }
        if (headerInfoPacket.GetMessageType() == MessageType::Snapshot && m_LastReceivedSnapshotGroup > headerInfoPacket.Group()) {
            it = m_PacketSegmentMap.erase(it);
            continue;
            //LOG_INFO("Deleted old entry");
        }
        if (currentVector.size() == groupSize) {
            std::sort(currentVector.begin(), currentVector.end());
            // Add the first packet in vector
            packet.ReconstructFromData(currentVector.at(0).second.get(), packet.HeaderSize());
            // Add the rest of the packets.
            int sizeOfData = 0;
            for (size_t i = 0; i < currentVector.size(); i++) {
                memcpy(&sizeOfData, currentVector.at(i).second.get(), sizeof(int));
                packet.WriteData(currentVector.at(i).second.get() + packet.HeaderSize(), sizeOfData - packet.HeaderSize());
            }
            if (headerInfoPacket.GetMessageType() == MessageType::Snapshot) {
                m_LastReceivedSnapshotGroup = packet.Group();
            }
            // No need to get next it as we are returning.
            m_PacketSegmentMap.erase(it);
            return true;
        } else {
            ++it;
        }
    }
    return false;
}
