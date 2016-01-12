#ifndef Events_SetSFXGain_h__
#define Events_SetSFXGain_h__

#include "Core/Event.h"

namespace Events
{

struct SetSFXGain : public Event
{
    float Gain;
};

}

#endif