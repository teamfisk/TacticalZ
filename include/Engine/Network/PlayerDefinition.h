#ifndef PlayerDefinition_h__
#define PlayerDefinition_h__
#include <string>

struct PlayerDefinition {
    unsigned int EntityID = -1;
    std::string Name = "";
    boost::asio::ip::udp::endpoint Endpoint;
};

#endif
