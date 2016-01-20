#include "Rendering/DrawScreenQuadPassState.h"


DrawScreenQuadPassState::DrawScreenQuadPassState()
{
    GLERROR("---");
    BindFramebuffer(0);
    GLERROR("---");
    Disable(GL_DEPTH_TEST);
    Disable(GL_CULL_FACE);
    Disable(GL_BLEND);
    glClearColor(0.f, 0.f, 0.f, 1.f);
   // ClearColor(glm::vec4(255.f / 255, 163.f / 255, 176.f / 255, 0.f));
    Clear(GL_COLOR_BUFFER_BIT);
}

DrawScreenQuadPassState::~DrawScreenQuadPassState()
{

}
