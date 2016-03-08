#ifndef Events_PlayerConnected
#define Events_PlayerConnected

#include "Core/EventBroker.h"

namespace Events
{

struct PlayerConnected : public Event
{
    std::string PlayerName = "";
    int PlayerID = -1;
};

}
#endif // !Events_PlayerConnected

