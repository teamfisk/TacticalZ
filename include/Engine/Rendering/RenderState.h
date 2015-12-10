#ifndef RenderState_h__
#define RenderState_h__

#include "../Common.h"
#include "../OpenGL.h"
#include "../GLM.h"

class RenderState 
{
public:
    RenderState();
    ~RenderState();
    bool Enable(GLenum GLEnable);
    bool CullFace(GLenum GlFaceToCull);
    bool ClearColor(glm::vec4 color);
    bool Clear(GLbitfield mask);
private:
    std::vector<GLenum> m_Enables;
    float m_preClearColor[4];
    GLenum m_preCullFace;
};
#endif