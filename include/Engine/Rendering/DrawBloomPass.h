#ifndef DrawBloomPass_h__
#define DrawBloomPass_h__

#include "IRenderer.h"
#include "DrawBloomPassState.h"
//#include "LightCullingPass.h" Finalpass om den skall skickas in
#include "FrameBuffer.h"
#include "ShaderProgram.h"
//#include "Util/UnorderedMapVec2.h"
#include "Texture.h"

class DrawBloomPass
{
public:
    DrawBloomPass(IRenderer* renderer, ConfigFile* config);
    ~DrawBloomPass() { }
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();
    void InitializeBuffers();
    void ClearBuffer();

    void FillGaussianBuffer(FrameBuffer* fb);

    void Draw(GLuint texture);
	void ChangeQuality(int quality);

    void OnWindowResize();

    //Getters
    //Return the blurred result of the texture that was sent into draw
	GLuint GaussianTexture() const {
		if (m_Quality == 0) {
			return m_BlackTexture->m_Texture;
		}
		else {
			return m_GaussianTexture_vert;
		}
	}



private:
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const;

    Texture* m_BlackTexture;
    Model* m_ScreenQuad;

    const IRenderer* m_Renderer;
	ConfigFile* m_Config;
    //const LightCullingPass* m_LightCullingPass 
	int m_Iterations = 7;
	int m_Quality = 3;

    GLuint m_GaussianTexture_horiz;
    GLuint m_GaussianTexture_vert;

    FrameBuffer m_GaussianFrameBuffer_horiz;
    FrameBuffer m_GaussianFrameBuffer_vert;

    ShaderProgram* m_GaussianProgram_horiz;
    ShaderProgram* m_GaussianProgram_vert;

};

#endif 