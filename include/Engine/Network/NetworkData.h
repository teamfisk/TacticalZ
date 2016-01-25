#ifndef NetworkData_h__
#define NetworkData_h__
#include <vector>

struct NetworkData {
    unsigned int TotalTime = 0;
    unsigned int TotalDataReceived = 0;
    unsigned int TotalDataSent = 0;
    unsigned int AmountOfMessagesReceived = 0;
    unsigned int AmountOfMessagesSent = 0;
    // Interval based
    unsigned int DataReceivedThisInterval = 0;
    unsigned int DataSentThisInterval = 0;
    // pair: first=reveived, second=send 
    std::vector<std::pair<unsigned int, unsigned int>> BandwidthBytes;
};

#endif
