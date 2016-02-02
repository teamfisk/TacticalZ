#include "Rendering/RenderState.h"

bool RenderState::Enable(GLenum cap)
{
    if (glIsEnabled(cap)) {
        //LOG_WARNING("Trying to enable somthing that is already enabled.");
        return false;
    }

    m_ResetFunctions.push_back(std::bind(glDisable, cap));
    glEnable(cap);
    return !GLERROR("Enable");
}

bool RenderState::Disable(GLenum cap)
{
    if (!glIsEnabled(cap)) {
        return false;
    }

    m_ResetFunctions.push_back(std::bind(glEnable, cap));
    glDisable(cap);
    return !GLERROR("Disable");
}

bool RenderState::CullFace(GLenum mode)
{
    if (!glIsEnabled(GL_CULL_FACE)) {
        LOG_ERROR("Setting GL_CULL_FACE without enabling it.");
        return false;
    }
    
    GLint original;
    glGetIntegerv(GL_CULL_FACE_MODE, &original);
    m_ResetFunctions.push_back(std::bind(glCullFace, original));
    glCullFace(mode);
    return !GLERROR("CullFace");
}

bool RenderState::ClearColor(glm::vec4 color)
{
    GLfloat original[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, &original[0]);
    m_ResetFunctions.push_back(std::bind(glClearColor, original[0], original[1], original[2], original[3]));
    glClearColor(color.r, color.g, color.b, color.a);
    return !GLERROR("ClearColor");
}

bool RenderState::BindFramebuffer(GLint framebuffer)
{
    GLint originalRead;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &originalRead);
    GLint originalDraw;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalDraw);
    m_ResetFunctions.push_back([originalRead, originalDraw]() {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, originalRead); 
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalDraw); 
    });
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    return !GLERROR("BindBuffer");
}


bool RenderState::BlendEquation(GLenum mode)
{
    GLint originalRGB;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &originalRGB);
    GLint originalAlpha;
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &originalAlpha);
    m_ResetFunctions.push_back(std::bind(glBlendEquationSeparate, originalRGB, originalAlpha));
    glBlendEquation(mode);
    return !GLERROR("BlendEquation");
}

bool RenderState::BlendFunc(GLenum sfactor, GLenum dfactor)
{
    GLint originalSrcRGB;
    glGetIntegerv(GL_BLEND_SRC_RGB, &originalSrcRGB);
    GLint originalSrcAlpha;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &originalSrcAlpha);
    GLint originalDestRGB;
    glGetIntegerv(GL_BLEND_DST_RGB, &originalDestRGB);
    GLint originalDestAlpha;
    glGetIntegerv(GL_BLEND_DST_ALPHA, &originalDestAlpha);
    m_ResetFunctions.push_back(std::bind(glBlendFuncSeparate, originalSrcRGB, originalSrcAlpha, originalDestRGB, originalDestAlpha));
    glBlendFunc(sfactor, dfactor);
    return !GLERROR("BlendFunc");
}


bool RenderState::StencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
{
    GLint originalSFail;
    glGetIntegerv(GL_STENCIL_FAIL, &originalSFail);
    GLint originalDPFail;
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &originalDPFail);
    GLint originalDPPass;
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &originalDPPass);
    m_ResetFunctions.push_back(std::bind(glStencilOp, originalSFail, originalDPFail, originalDPPass));
    glStencilOp(sfail, dpfail, dppass);
    return !GLERROR("StencilOp");
}


bool RenderState::StencilFunc(GLenum func, GLint ref, GLuint mask)
{
    GLint originalFunc;
    glGetIntegerv(GL_STENCIL_FUNC, &originalFunc);
    GLint originalRef;
    glGetIntegerv(GL_STENCIL_REF, &originalRef);
    GLint originalMask;
    glGetIntegerv(GL_STENCIL_VALUE_MASK, &originalMask);
    m_ResetFunctions.push_back(std::bind(glStencilFunc, originalFunc, originalRef, originalMask));
    glStencilFunc(func, ref, mask);
    return !GLERROR("StencilFunc");
}



bool RenderState::StencilMask(GLuint mask)
{
    GLint originalMask;
    glGetIntegerv(GL_STENCIL_WRITEMASK, &originalMask);
    m_ResetFunctions.push_back(std::bind(glStencilMask, mask));
    glStencilMask(mask);
    return !GLERROR("StencilMask");
}

bool RenderState::DepthMask(GLboolean flag)
{
    GLboolean original;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &original);
    m_ResetFunctions.push_back(std::bind(glDepthMask, original));
    glDepthMask(flag);
    return !GLERROR("DepthMask");
}

bool RenderState::DepthFunc(GLenum func)
{
	GLint original;
	glGetIntegerv(GL_DEPTH_FUNC, &original);
	m_ResetFunctions.push_back(std::bind(glDepthFunc, original));
	glDepthFunc(func);
	return !GLERROR("DepthFunc");
}

bool RenderState::AlphaFunc(GLenum func, GLclampf thresholder)
{
	GLint originalFunc;
	glGetIntegerv(GL_ALPHA_TEST_FUNC, &originalFunc);
	GLint originalRef;
	glGetIntegerv(GL_ALPHA_TEST_REF, &originalRef);
	m_ResetFunctions.push_back(std::bind(glAlphaFunc, originalFunc, originalRef));
	glAlphaFunc(func, thresholder);
	return !GLERROR("AlphaFunc");
}

bool RenderState::PolygonMode(GLenum face, GLenum mode)
{
    GLint originalMode[2];
    glGetIntegerv(GL_POLYGON_MODE, &originalMode[0]);
    if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
        m_ResetFunctions.push_back(std::bind(glPolygonMode, GL_FRONT, originalMode[0]));
    }
    if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
        m_ResetFunctions.push_back(std::bind(glPolygonMode, GL_BACK, originalMode[1]));
    }
    glPolygonMode(face, mode);
    return !GLERROR("RenderState::PolygonMode");
}

RenderState::~RenderState()
{
    for (auto& f : boost::adaptors::reverse(m_ResetFunctions)) {
        f();
    }
}

