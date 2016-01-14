#ifndef Events_PlayBackgroundMusic_h__
#define Events_PlayBackgroundMusic_h__

#include <string>
#include "Core/Entity.h"
#include "Core/Event.h"

namespace Events
{
// Play a sound that will be heared the same anywhere
struct PlayBackgroundMusic : public Event
{
    std::string FilePath = "";
};

}

#endif