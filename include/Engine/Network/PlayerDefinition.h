#ifndef PlayerDefinition_h__
#define PlayerDefinition_h__
#include <string>
#include "../Core/Entity.h"
#include <boost/asio.hpp>

struct PlayerDefinition {
    ::EntityID EntityID = EntityID_Invalid;
    std::string Name = "";
    boost::asio::ip::udp::endpoint Endpoint;
    unsigned int PacketID;
    std::clock_t StopTime;
    boost::asio::ip::address Address;
    unsigned short Port;
    // use for tcp connections
    boost::shared_ptr<boost::asio::ip::tcp::socket> TCPSocket;
};

#endif
