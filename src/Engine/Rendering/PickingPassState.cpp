#include "Rendering/PickingPassState.h"


PickingPassState::PickingPassState(GLuint frameBuffer)
{
    GLERROR("---2");
    BindFramebuffer(frameBuffer);
    GLERROR("---3");
    Enable(GL_DEPTH_TEST);
    Enable(GL_CULL_FACE);

    glm::vec4 clearColor = glm::vec4(0.f);
    //ClearColor(clearColor);
    //Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

PickingPassState::~PickingPassState()
{

}
