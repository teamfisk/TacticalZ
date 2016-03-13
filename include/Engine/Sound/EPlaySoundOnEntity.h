#ifndef Events_PlaySoundOnEntity_h__
#define Events_PlaySoundOnEntity_h__

#include <string>
#include "Core/Entity.h"
#include "Core/Event.h"

namespace Events
{

// Plays a sound on an entity with a SoundEmitter component attached.
// Sound behavior is thereby specified in the SoundEmitter component.
struct PlaySoundOnEntity : public Event
{
    EntityWrapper Emitter = EntityWrapper::Invalid;
    float Gain = 1.f;
    std::string FilePath = "";
};

}

#endif