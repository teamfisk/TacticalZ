#include "Rendering/TextPassState.h"


TextPassState::TextPassState(GLuint frameBuffer)
{
    BindFramebuffer(frameBuffer);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

TextPassState::~TextPassState()
{

}
