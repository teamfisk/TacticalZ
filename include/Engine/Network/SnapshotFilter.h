#ifndef SnapshotFilter_h__
#define SnapshotFilter_h__

#include "../Core/EntityWrapper.h"
#include "../Core/ComponentWrapper.h"

class SnapshotFilter
{
public:
    // Filters an incoming snapshot. 
    // Modify the component and return true if the component snapshot should be applied. 
    // Otherwise return false and it will be ignored.
    virtual bool FilterComponent(EntityWrapper entity, SharedComponentWrapper& component) 
    {
        return true;
    }
};

#endif