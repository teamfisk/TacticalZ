#include "Rendering/DrawBloomPassState.h"


DrawBloomPassState::DrawBloomPassState()
{
    //BindFramebuffer(0);
    Disable(GL_BLEND);
    Disable(GL_DEPTH_TEST);
    Disable(GL_CULL_FACE);
}

DrawBloomPassState::~DrawBloomPassState()
{

}
