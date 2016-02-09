#ifndef DrawBloomPass_h__
#define DrawBloomPass_h__

#include "IRenderer.h"
#include "DrawBloomPassState.h"
//#include "LightCullingPass.h" Finalpass om den skall skickas in
#include "FrameBuffer.h"
#include "ShaderProgram.h"
//#include "Util/UnorderedMapVec2.h"
#include "Texture.h"
#include "imgui/imgui.h"

class DrawBloomPass
{
public:
    DrawBloomPass(IRenderer* renderer /* ,Texture or finalpass*/ );
    ~DrawBloomPass() { }
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();
    void InitializeBuffers();
    void ClearBuffer();

    void FillGaussianBuffer(FrameBuffer* fb);

    void Draw(GLuint texture);

    //Getters
    //Return the blurred result of the texture that was sent into draw
    GLuint GaussianTexture() const { return m_FinalBlurTexture; }


private:
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const;
    void BlurOneMipMapLevel(GLuint* texture, GLint iterations, GLint mipMapLevel);

    Texture* m_WhiteTexture;
    Model* m_ScreenQuad;

    const IRenderer* m_Renderer;
    //const LightCullingPass* m_LightCullingPass 
    //GLuint m_iterations = 5;
    GLint m_DebugTextureToDraw = 0;
    GLint m_Iterations = 3;

    float m_ScreenSizes[10];

    GLuint m_FinalBlurTexture;
    GLuint m_GaussianTexture_horiz;
    GLuint m_GaussianTexture_vert;

    FrameBuffer m_GaussianFrameBuffer_horiz;
    FrameBuffer m_GaussianFrameBuffer_vert;
    FrameBuffer m_BlurredFinalTexture;

    ShaderProgram* m_GaussianProgram_horiz;
    ShaderProgram* m_GaussianProgram_vert;
    ShaderProgram* m_GaussianProgram_both;
    ShaderProgram* m_CombineProgram;

};

#endif 