#ifndef PickingPassState_h__
#define PickingPassState_h__

#include "Rendering/RenderState.h"

class PickingPassState : public RenderState
{
public:
    PickingPassState(GLuint frameBuffer);
    ~PickingPassState();

private:
};

#endif