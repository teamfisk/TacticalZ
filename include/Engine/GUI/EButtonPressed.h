#ifndef Events_ButtonPressed_h__
#define Events_ButtonPressed_h__

#include "Core/Event.h"

namespace Events
{

struct ButtonPressed : public Event {
    std::string EntityName = "DEFAULT STRING USED";
};

}

#endif