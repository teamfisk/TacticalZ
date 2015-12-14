#ifndef RenderState_h__
#define RenderState_h__

#include <functional>
#include "../Common.h"
#include "../OpenGL.h"
#include "../GLM.h"

class RenderState 
{
public:
    RenderState() = default;
    ~RenderState();

    bool Enable(GLenum cap);
    bool Disable(GLenum cap);
    bool CullFace(GLenum mode);
    bool ClearColor(glm::vec4 color);
    bool Clear(GLbitfield mask);
    bool BindFramebuffer(GLint framebuffer);
    bool BlendEquation(GLenum mode);
    bool BlendFunc(GLenum sfactor, GLenum dfactor);

private:
    std::vector<std::function<void(void)>> m_ResetFunctions;
};
#endif