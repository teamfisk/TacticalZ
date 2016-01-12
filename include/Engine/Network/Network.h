#ifndef Network_h__
#define Network_h__

#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Network/Packet.h"

#define MAXCONNECTIONS 8
#define INPUTSIZE 4097

class Network
{
public:
    virtual ~Network() { };
    virtual void Start(World* m_world, EventBroker *eventBroker) = 0;
    virtual void Update() = 0;
    virtual void Close() = 0;
};

#endif