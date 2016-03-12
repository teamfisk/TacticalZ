#ifndef Events_PlayAnnouncerVoice_h__
#define Events_PlayAnnouncerVoice_h__

#include <string>
#include "../Engine/Core/EventBroker.h"

namespace Events
{

struct PlayAnonuncerVoice : public Event
{
    std::string FilePath = "";
};

}

#endif
