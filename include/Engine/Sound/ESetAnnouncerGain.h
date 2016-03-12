#ifndef Events_SetAnnouncerGain_h__
#define Events_SetAnnouncerGain_h__

#include "../Engine/Core/EventBroker.h"

namespace Events
{

struct  SetAnnouncerGain : public Event
{
    float Gain = 1;
};

}

#endif // 
