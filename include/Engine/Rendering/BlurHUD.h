#ifndef BlurHUD_h__
#define BlurHUD_h__

#include "IRenderer.h"
#include "DrawBloomPassState.h"
//#include "LightCullingPass.h" Finalpass om den skall skickas in
#include "FrameBuffer.h"
#include "ShaderProgram.h"
//#include "Util/UnorderedMapVec2.h"
#include "Texture.h"

class BlurHUD
{
public:
    BlurHUD(IRenderer* renderer);
    ~BlurHUD() { }
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();
    void InitializeBuffers();
    void ClearBuffer();

    void FillGaussianBuffer(FrameBuffer* fb);

    GLuint Draw(GLuint texture, RenderScene& scene);

    void OnWindowResize();
    void FillStencil(RenderScene& scene);
    GLuint CombineTextures(GLuint texture1, GLuint texture2);

    //Getters
    //Return the blurred result of the texture that was sent into draw
	GLuint GaussianTexture() const { 
		if (m_Quality == 0) { 
			return m_BlackTexture->m_Texture;
		} else { 
			return m_GaussianTexture_vert;
		} 
	}


private:
    Texture* m_BlackTexture;
    Model* m_ScreenQuad;

    const IRenderer* m_Renderer;
	ConfigFile* m_Config;
    //const LightCullingPass* m_LightCullingPass 
    int m_Iterations = 3;
	int m_Quality = 0;
    float m_BlurQuality = 2.f;

    GLuint m_GaussianTexture_horiz = 0;
    GLuint m_GaussianTexture_vert = 0;
    GLuint m_DepthStencil_horiz = 0;
    GLuint m_DepthStencil_vert = 0;
    GLuint m_CombinedTexture = 0;

    FrameBuffer m_GaussianFrameBuffer_horiz;
    FrameBuffer m_GaussianFrameBuffer_vert;
    FrameBuffer m_CombinedTextureBuffer;

    ShaderProgram* m_GaussianProgram_horiz;
    ShaderProgram* m_GaussianProgram_vert;
    ShaderProgram* m_FillDepthStencilProgram;
    ShaderProgram* m_CombineTexturesProgram;

};

#endif 