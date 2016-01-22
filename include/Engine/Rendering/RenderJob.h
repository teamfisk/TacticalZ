#ifndef RenderJob_h__
#define RenderJob_h__

#include <cstdint>

#include "../Common.h"
#include "../GLM.h"
#include "../Core/ComponentWrapper.h"
#include "RenderQueue.h"


struct RenderJob
{
    friend class RenderQueue;

public:
    float Depth;

protected:
    uint64_t Hash;

    virtual void CalculateHash() = 0;

    bool operator<(const RenderJob& rhs)
    {
        return this->Hash < rhs.Hash;
    }

};

#endif