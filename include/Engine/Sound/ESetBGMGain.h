#ifndef Events_SetBGMGain_h__
#define Events_SetBGMGain_h__

#include "Core/Event.h"

namespace Events
{

struct SetBGMGain : public Event
{
    float Gain;
};

}

#endif