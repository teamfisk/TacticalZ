#include "Rendering/RenderState.h"

RenderState::RenderState()
{
}

bool RenderState::Enable(GLenum GLEnable)
{
    if(glIsEnabled(GLEnable))
    {
        LOG_WARNING("Trying to enable somthing that is already enabled.");
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
    if(a == GL_BACK)
    {
        LOG_INFO("Setting Cullface to back, unessesary since this is already default.");
    }
    m_preCullFace = a;
    glCullFace(GLCullFace);
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

RenderState::~RenderState()
{
    //Set cullface to default
    glCullFace(m_preCullFace);

    //Set color to default
    glClearColor(m_preClearColor[0], m_preClearColor[1], m_preClearColor[2], m_preClearColor[3]);

    //Disable Enables
    for (auto i : m_Enables)
    {
        glDisable(i);
    }

    m_Enables.clear();
}

