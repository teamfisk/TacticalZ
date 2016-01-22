#ifndef Network_h__
#define Network_h__

#include <ctime>

#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Network/Packet.h"
#include "Network/NetworkData.h"
#include <fstream>
#include <iostream>

#define MAXCONNECTIONS 8
#define INPUTSIZE 4097
#define TIMEOUTMS 15000

class Network
{
public:
    virtual ~Network() { };
    virtual void Start(World* m_world, EventBroker *eventBroker) = 0;
    virtual void Update() = 0;
protected:
    // For Debug 
    bool isReadingData = false;
    NetworkData m_NetworkData;
    unsigned int m_SaveDataIntervalMs = 1000;
    std::clock_t m_SaveDataTimer;
    void saveToFile();
    void updateNetworkData();
};

#endif