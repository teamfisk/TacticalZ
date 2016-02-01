#ifndef PlayerDefinition_h__
#define PlayerDefinition_h__
#include <string>
#include "../Core/Entity.h"

struct PlayerDefinition {
    ::EntityID EntityID = EntityID_Invalid;
    std::string Name = "";
    boost::asio::ip::udp::endpoint Endpoint;
    unsigned int PacketID;
    std::clock_t StopTime;
};

#endif
