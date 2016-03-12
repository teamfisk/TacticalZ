#ifndef Events_ChangeBGM_h__
#define Events_ChangeBGM_h__

#include <string>
#include "../Engine/Core/EventBroker.h"

namespace Events
{

struct ChangeBGM : Event
{
    std::string FilePath = "";
};

}
#endif // Events_ChangeBGM_h__
