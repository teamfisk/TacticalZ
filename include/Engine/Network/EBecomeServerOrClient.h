#ifndef Events_BecomeServerOrClient_h__
#define Events_BecomeServerOrClient_h__

#include "Core/EventBroker.h"

namespace Events 
{

struct BecomeServer : public Event { };

struct BecomeClient : public Event { };

}

#endif
