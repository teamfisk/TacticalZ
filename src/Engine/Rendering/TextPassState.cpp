#include "Rendering/TextPassState.h"


TextPassState::TextPassState(GLuint frameBuffer)
{
    BindFramebuffer(frameBuffer);
    Enable(GL_BLEND);
    Disable(GL_CULL_FACE);
    Enable(GL_DEPTH_TEST);
    BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Enable(GL_ALPHA_TEST);
    AlphaFunc(GL_GEQUAL, 0.05f);
    
}

TextPassState::~TextPassState()
{

}
