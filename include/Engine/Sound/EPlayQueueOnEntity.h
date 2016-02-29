#ifndef Events_PlayQueueOnEntity_h__
#define Events_PlayQueueOnEntity_h__

#include "../Core/Event.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct PlayQueueOnEntity : public Event
{
    EntityWrapper Emitter;
    std::vector<std::string> FilePaths;
};

}

#endif
