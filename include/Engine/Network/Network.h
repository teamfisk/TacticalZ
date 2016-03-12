#ifndef Network_h__
#define Network_h__

#include <ctime>

#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Network/Packet.h"
#include "Network/NetworkData.h"
#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"
#include <fstream>
#include <iostream>

#define BUFFERSIZE 32000
typedef unsigned int PlayerID;
typedef unsigned int PacketID;

class Network
{
public:
    Network(World* world, EventBroker* eventBroker);
    virtual ~Network() { };

    virtual void Update() = 0;

protected:
    World* m_World;
    EventBroker* m_EventBroker;

    // For Debug 
    bool isReadingData = false;
    NetworkData m_NetworkData;
    unsigned int m_SaveDataIntervalMs = 1000;
    std::clock_t m_SaveDataTimer;
    unsigned int m_MaxConnections;
    double m_TimeoutMs;
    void logSentData(int bytesSent);
    void logReceivedData(int bytesReceived);
    void saveToFile();
    void updateNetworkData();
    void popNetworkSegmentOfHeader(Packet& packet);
    void removeWorld();
};

#endif