#ifndef PlayerDefinition_h__
#define PlayerDefinition_h__
#include <string>
#include "../Core/Entity.h"

struct PlayerDefinition {
    ::EntityID EntityID = EntityID_Invalid;
    std::string Name = "";
    boost::asio::ip::udp::endpoint Endpoint;
    // The ID of the last sent packet. (Local sequence number)
    unsigned int PacketID;
    // The ID of the last received packet. (Remote sequence number)
    // This is sent as the ackNumber.
    unsigned int LastPacketReceivedID;
    // The bit pattern of the last 32 received packets.
    unsigned int AckBitField;
    std::clock_t StopTime;
};

#endif
