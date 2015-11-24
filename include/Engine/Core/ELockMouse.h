#ifndef Events_LockMouse_h__
#define Events_LockMouse_h__

#include "Core/EventBroker.h"

namespace Events
{

/** Called to lock mouse to the middle of the window. */
struct LockMouse : Event { };
/** Called to unlock mouse from the middle of the window. */
struct UnlockMouse : Event { };

}

#endif
