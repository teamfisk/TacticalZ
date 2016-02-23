#ifndef Events_DisplayServerlist_h__
#define Events_DisplayServerlist_h__

#include <string>
#include <vector>
#include "Core/Event.h"

struct ServerInfo
{
    ServerInfo(std::string address, int port, std::string name, int players)
    {
        Address = address; Port = port; Name = name; PlayersConnected = players;
    }
    std::string Address = "";
    int Port = 0;
    std::string Name = "";
    int PlayersConnected = 0;
};

namespace Events
{

struct DisplayServerlist : public Event 
{
    std::vector<ServerInfo> Serverlist;
};

}
#endif
