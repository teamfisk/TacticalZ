#ifndef EResolutionChanged_h__
#define EResolutionChanged_h__

#include "../Core/Event.h"
#include "../Core/Util/Rectangle.h"

namespace Events
{

// Fired when the framebuffer size changes
struct ResolutionChanged : Event
{
    Rectangle OldResolution;
    Rectangle NewResolution;
};

}

#endif