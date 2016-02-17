#ifndef Events_ButtonClicked_h__
#define Events_ButtonClicked_h__

#include "Core/Event.h"

namespace Events
{

struct ButtonClicked : public Event {
    std::string EntityName = "DEFAULT STRING USED";
};

}

#endif