#ifndef Events_KillDeath_h__
#define Events_KillDeath_h__

#include "Core/EventBroker.h"

typedef unsigned int PlayerID;

namespace Events
{

struct KillDeath : public Event 
{
    int CasualtyTeam;
    PlayerID Casualty = -1;
    std::string CasualtyName;

    //1 = assault, 2 = defender, 3 = sniper
    int CasualtyClass;

    int KillerTeam;
    PlayerID Killer = -1;
    std::string KillerName;

    //1 = assault, 2 = defender, 3 = sniper
    int KillerClass;
};

}

#endif