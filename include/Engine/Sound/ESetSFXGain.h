#ifndef Events_SetSFXGain_h__
#define Events_SetSFXGain_h__

#include "Core/Event.h"

namespace Events
{
// Set the "volume" for all effect sounds
struct SetSFXGain : public Event
{
    float Gain;
};

}

#endif