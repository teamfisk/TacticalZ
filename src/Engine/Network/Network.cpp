#include "Network/Network.h"

Network::Network(World* world, EventBroker* eventBroker) 
    : m_World(world)
    , m_EventBroker(eventBroker)
{
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    m_MaxConnections = config->Get<int>("Networking.MaxConnections", 8);
    m_TimeoutMs = config->Get<int>("Networking.TimeoutMs", 20000);
}

void Network::Update()
{ 
    updateNetworkData();
}

void Network::logSentData(int bytesSent)
{ 

}

void Network::logReceivedData(int bytesReceived)
{ 
    // Network Debug data
    if (isReadingData) {
        m_NetworkData.TotalDataReceived += bytesReceived;
        m_NetworkData.DataReceivedThisInterval += bytesReceived;
        m_NetworkData.AmountOfMessagesReceived++;
    }
}

void Network::saveToFile()
{
    std::ofstream outfile;
    time_t t = time(0);
    // get time now
    struct tm * now = localtime(&t);
    // Get current time and date
    std::string dateAndTime = "BandwidthData - " + std::to_string(now->tm_year + 1900) + '-'
        + std::to_string(now->tm_mon + 1) + '-'
        + std::to_string(now->tm_mday) + '_'
        + std::to_string(now->tm_hour) + "h."
        + std::to_string(now->tm_min) + "m."
        + std::to_string(now->tm_sec) + 's';

    outfile.open(dateAndTime + ".csv");
    outfile << "Total time," + std::to_string(m_NetworkData.TotalTime) + "\n";
    outfile << "Total data received," + std::to_string(m_NetworkData.TotalDataReceived) + "\n";
    outfile << "Total data sent," + std::to_string(m_NetworkData.TotalDataSent) + "\n";
    outfile << "Total messages received," + std::to_string(m_NetworkData.AmountOfMessagesReceived) + "\n";
    outfile << "Total messages sent," + std::to_string(m_NetworkData.AmountOfMessagesSent) + "\n";

    double messagesReceivedPerSec = m_NetworkData.AmountOfMessagesReceived / (m_NetworkData.TotalTime / 1000);
    double messagesSentPerSec = m_NetworkData.AmountOfMessagesSent / (m_NetworkData.TotalTime / 1000);
    double dataReceivedPerSec = m_NetworkData.TotalDataReceived / (m_NetworkData.TotalTime / 1000);
    double dataSentPerSec = m_NetworkData.TotalDataSent / (m_NetworkData.TotalTime / 1000);
    outfile << "Avarage messages received / s: " + std::to_string(messagesReceivedPerSec) + "\n";
    outfile << "Avarage messages sents / s: " + std::to_string(messagesSentPerSec) + "\n";
    outfile << "Avarage data received B/s: " + std::to_string(dataReceivedPerSec) + "\n";
    outfile << "Avarage data sents B/s: " + std::to_string(dataSentPerSec) + "\n";
    
    outfile << "time, avg receive B, avg send B\n";
    for (int i = 0; i < m_NetworkData.BandwidthBytes.size(); i++) {
        outfile << std::to_string(i) + ",";
        outfile << std::to_string(m_NetworkData.BandwidthBytes[i].first) + ",";
        outfile << std::to_string(m_NetworkData.BandwidthBytes[i].second) + "\n";
    }
    outfile.close();

}

void Network::updateNetworkData()
{
    std::clock_t currentTime = std::clock();
    // Send snapshot
    if (m_SaveDataIntervalMs < (1000 * (currentTime - m_SaveDataTimer) / (double)CLOCKS_PER_SEC)) {
        // Set values
        m_NetworkData.TotalTime += (1000 * (currentTime - m_SaveDataTimer) / (double)CLOCKS_PER_SEC);
        m_NetworkData.BandwidthBytes.push_back(std::pair<size_t, size_t>(m_NetworkData.DataReceivedThisInterval, m_NetworkData.DataSentThisInterval));
        // Reset interval stuff
        m_SaveDataTimer = std::clock();
        m_NetworkData.DataSentThisInterval = 0;
        m_NetworkData.DataReceivedThisInterval = 0;
    }
}
