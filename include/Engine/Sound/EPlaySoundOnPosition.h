#ifndef Events_PlaySoundOnPosition_h__
#define Events_PlaySoundOnPosition_h__

#include <string>
#include <glm/common.hpp>
#include "Core/Event.h"

namespace Events
{

// Plays a sound on a given position
struct PlaySoundOnPosition : public Event
{
    glm::vec3 Position = glm::vec3(0);
    std::string FilePath = "";
    float Gain = 1;
    float Pitch = 1;
    bool Loop = false;
    float MaxDistance = 20;
    float RollOffFactor = 1;
    float ReferenceDistance = 1;
};

}

#endif