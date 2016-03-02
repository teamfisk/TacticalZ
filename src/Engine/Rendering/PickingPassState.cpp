#include "Rendering/PickingPassState.h"


PickingPassState::PickingPassState(GLuint frameBuffer)
{
    GLERROR("PRE");
    BindFramebuffer(frameBuffer);
    GLERROR("Bind Framebuffer");
    Enable(GL_DEPTH_TEST);
    Enable(GL_CULL_FACE);
    Disable(GL_BLEND);
    glm::vec4 clearColor = glm::vec4(0.f);
    //ClearColor(clearColor);
    //Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLERROR("END");
	
}

PickingPassState::~PickingPassState()
{

}
