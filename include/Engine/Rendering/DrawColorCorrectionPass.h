#ifndef DrawColorCorrectionPass_h__
#define DrawColorCorrectionPass_h__

#include "IRenderer.h"
#include "DrawScreenQuadPassState.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
//#include "Util/UnorderedMapVec2.h"
#include "Texture.h"

class DrawColorCorrectionPass
{
public:
    DrawColorCorrectionPass(IRenderer* renderer);
    ~DrawColorCorrectionPass() { }
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();

    void Draw(GLuint sceneTexture, GLuint bloomTexture);
private:
    const IRenderer* m_Renderer;

    ShaderProgram* m_ColorCorrectionProgram;

    Model* m_ScreenQuad;
    GLfloat m_Exposure;
};

#endif 