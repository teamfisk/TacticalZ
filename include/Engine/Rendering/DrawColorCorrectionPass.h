#ifndef DrawColorCorrectionPass_h__
#define DrawColorCorrectionPass_h__

#include "IRenderer.h"
#include "DrawScreenQuadPassState.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
//#include "Util/UnorderedMapVec2.h"
#include "Texture.h"
#include "imgui/imgui.h"

class DrawColorCorrectionPass
{
public:
    DrawColorCorrectionPass(IRenderer* renderer);
    ~DrawColorCorrectionPass() { }
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();

    void Draw(GLuint sceneTexture, GLuint bloomTexture, GLuint sceneTextureLowRes, GLuint bloomTextureLowRes, GLfloat gamma, GLfloat exposure);
private:
    const IRenderer* m_Renderer;

    ShaderProgram* m_ColorCorrectionProgram;

    Model* m_ScreenQuad;
};

#endif 