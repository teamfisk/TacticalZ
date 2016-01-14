#ifndef Events_PlaySoundOnEntity_h__
#define Events_PlaySoundOnEntity_h__

#include <string>
#include "Core/Entity.h"
#include "Core/Event.h"

namespace Events
{

// Plays a sound on an entity with a SoundEmitter component attached.
// Sound behavior is thereby specified in the SoundEmitter component.
// ?(???)??
struct PlaySoundOnEntity : public Event
{
    EntityID EmitterID = 0;
    std::string FilePath = "";
};

}

#endif