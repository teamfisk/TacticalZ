#ifndef PlayerDefinition_h__
#define PlayerDefinition_h__
#include <string>

struct PlayerDefinition {
    int EntityID = -1;
    std::string Name = "";
    boost::asio::ip::udp::endpoint Endpoint;
    unsigned int PacketID;
    std::clock_t StopTime;
};

#endif
