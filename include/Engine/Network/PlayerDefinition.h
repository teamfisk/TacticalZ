#ifndef PlayerDefinition_h__
#define PlayerDefinition_h__
#include <string>

struct PlayerDefinition {
    unsigned int EntityID;
    std::string Name;
    boost::asio::ip::udp::endpoint Endpoint;
};

#endif
