#include "Rendering/PickingPassState.h"


PickingPassState::PickingPassState()
{
    Enable(GL_DEPTH_TEST);
    Enable(GL_CULL_FACE);
    CullFace(GL_BACK);

    glm::vec4 clearColor = glm::vec4(0.f);
    ClearColor(clearColor);
    Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

