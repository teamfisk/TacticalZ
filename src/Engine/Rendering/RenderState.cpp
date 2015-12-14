#include "Rendering/RenderState.h"

RenderState::RenderState()
{

}

bool RenderState::Enable(GLenum GLEnable)
{
    if(glIsEnabled(GLEnable))
    {
        return false;
    }
    m_Enables.push_back(GLEnable);
    glEnable(GLEnable);
    if (GLERROR("RenderState::Enable"))
    {
        return false;
    }
    return true;
}

bool RenderState::CullFace(GLenum GLCullFace)
{
    if(!glIsEnabled(GL_CULL_FACE))
    {
        LOG_ERROR("Setting GL_CULL_FACE without enabling it.");
        return false;
    }
    
    GLint a;
    glGetIntegerv(GL_CULL_FACE_MODE, &a);
    if(a != GL_BACK)
    {
        //LOG_INFO("Setting Cullface to back, unessesary since this is already default.");
        glCullFace(GLCullFace);
    }
    if (GLERROR("RenderState::CullFace"))
    {
        return false;
    }
    return true;
}

bool RenderState::ClearColor(glm::vec4 color)
{
    glGetFloatv(GL_COLOR_CLEAR_VALUE, &m_preClearColor[0]);
    glClearColor(color.r, color.g, color.b, color.a);
    if (GLERROR("RenderState::ClearColor")) {
        return false;
    }
    return true;
}

bool RenderState::Clear(GLbitfield mask)
{
    glClear(mask);
    if (GLERROR("RenderState::Clear")) {
        return false;
    }
    return true;
}

bool RenderState::BindBuffer(GLint buffer)
{
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_preBuffer);
    if (buffer == m_preBuffer)
    {
        return true;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, buffer);
    if (GLERROR("RenderState::BindBuffer"))
    {
        printf("BufferID: %i\npreBufferID: %i\n", buffer, m_preBuffer);
        return false;
    }
    return true;
}

RenderState::~RenderState()
{
    GLERROR("RenderState::~RenderState Pre");
    GLint n_buffer = -1;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &n_buffer);

    //Set cullface to default
    if (glIsEnabled(GL_CULL_FACE)) {
        glCullFace(GL_BACK);
    }
    GLERROR("RenderState::~RenderState glCullFace");

    //Set color to default
    glClearColor(m_preClearColor[0], m_preClearColor[1], m_preClearColor[2], m_preClearColor[3]);
    GLERROR("RenderState::~RenderState glClearColor");

    //Disable Enables
    for (auto i : m_Enables)
    {
        glDisable(i);
    }
    GLERROR("RenderState::~RenderState glDisable");

    if(m_preBuffer != 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    m_Enables.clear();
    GLERROR("RenderState::~RenderState glBindFramebuffer");
}

