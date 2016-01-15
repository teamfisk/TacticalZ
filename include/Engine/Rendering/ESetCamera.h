#ifndef Events_SetCamera_h__
#define Events_SetCamera_h__

#include "../Core/EventBroker.h"
#include "../Core/Entity.h"
#include <string.h>

namespace Events
{

struct SetCamera : Event
{
public:
    SetCamera() { };
    std::string Name;

private:

};

}

#endif
