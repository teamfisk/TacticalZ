#ifndef DrawFinalPassState_h__
#define DrawFinalPassState_h__

#include "Rendering/RenderState.h"

class DrawFinalPassState : public RenderState
{
public:
    DrawFinalPassState(GLuint frameBuffer);
    ~DrawFinalPassState();
private:

};

class DrawStencilState : public RenderState
{
public:
    DrawStencilState(GLuint frameBuffer);
    ~DrawStencilState();
};

#endif