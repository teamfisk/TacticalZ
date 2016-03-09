#ifndef Events_ConnectRequest_h__
#define Events_ConnectRequest_h__

#include "Core/EventBroker.h"

namespace Events
{

struct ConnectRequest : public Event
{
    std::string IP = "";
    int Port = 0;
};

}
#endif

