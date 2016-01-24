#ifndef DrawScreenQuadPass_h__
#define DrawScreenQuadPass_h__

#include "IRenderer.h"
#include "DrawScreenQuadPassState.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
//#include "Util/UnorderedMapVec2.h"
#include "Texture.h"

class DrawScreenQuadPass
{
public:
    DrawScreenQuadPass(IRenderer* renderer);
    ~DrawScreenQuadPass() { }
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();

    void Draw(GLuint texture);
private:
    const IRenderer* m_Renderer;

    ShaderProgram* m_DrawQuadProgram;

    Model* m_ScreenQuad;
};

#endif 