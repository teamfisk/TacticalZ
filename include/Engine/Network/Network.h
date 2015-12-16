#ifndef Network_h__
#define Network_h__

#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Network/Package.h"

class Network
{
public:
    Network();
    ~Network();
    virtual void Start(World* m_world, EventBroker *eventBroker);
    virtual void Update();
protected:

};

#endif