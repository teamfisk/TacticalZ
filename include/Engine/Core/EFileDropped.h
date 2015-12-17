#ifndef EFileDropped_h__
#define EFileDropped_h__

#include "EventBroker.h"

namespace Events
{

struct FileDropped : Event
{
    std::string Path;
};

}

#endif