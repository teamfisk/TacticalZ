#ifndef NetworkData_h__
#define NetworkData_h__
#include <vector>

struct NetworkData {
    double TotalTime = 0;
    size_t TotalDataReceived = 0;
    size_t TotalDataSent = 0;
    size_t AmountOfMessagesReceived = 0;
    unsigned int AmountOfMessagesSent = 0;
    // Interval based
    size_t DataReceivedThisInterval = 0;
    size_t DataSentThisInterval = 0;
    // pair: first=reveived, second=send 
    std::vector<std::pair<size_t, size_t>> BandwidthBytes;
};

#endif