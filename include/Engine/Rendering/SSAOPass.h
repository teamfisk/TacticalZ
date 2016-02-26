#ifndef SSAOPass_h__
#define SSAOPass_h__

#include "IRenderer.h"
#include "SSAOPassState.h"
//#include "LightCullingPass.h" Finalpass om den skall skickas in
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "DrawBloomPass.h"
//#include "Util/UnorderedMapVec2.h"
#include "Texture.h"

class SSAOPass
{
public:
	SSAOPass(IRenderer* renderer, ConfigFile* config);
	~SSAOPass() { };

	void ChangeQuality(int quality);

	void Draw(GLuint depthBuffer, Camera* camera);
	void Setting(float radius, float bias, float contrast, float intensityScale, int numOfSamples, int numOfTurns, int iterations, int quality);
	void ClearBuffer();
	void OnWindowResize();

	//Return the SSAO of the texture sent to Draw
	GLuint SSAOTexture() const { 
		if (m_Quality == 0) {
			return m_WhiteTexture->m_Texture;
		} else {
			return m_GaussianTexture_vert;
		}
	}

	int TextureQuality() const {
		if (m_Quality == 0) {
			return 13;
		} else {
			return m_TextureQuality;
		}
	}

private:
	void InitializeTexture();
	void InitializeFrameBuffer();
	void InitializeShaderProgram();
	void InitializeBuffer();

	void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const;

	//void blurHorizontal(GLuint depthBuffer);
	//void blurVertical(GLuint depthBuffer);

	Model* m_ScreenQuad;

	const IRenderer* m_Renderer;
	ConfigFile* m_Config;

	float m_Radius;
	float m_Bias;
	float m_Contrast;
	float m_IntensityScale;
	int m_NumOfSamples;
	int m_NumOfTurns;
	int m_Iterations;
	int m_TextureQuality;
	int m_Quality;

	Texture* m_WhiteTexture;

	GLuint m_SSAOTexture;
	FrameBuffer m_SSAOFramBuffer;

	GLuint m_SSAOViewSpaceZTexture;
	FrameBuffer m_SSAOViewSpaceZFramBuffer;

	GLuint m_GaussianTexture_horiz;
	GLuint m_GaussianTexture_vert;

	FrameBuffer m_GaussianFrameBuffer_horiz;
	FrameBuffer m_GaussianFrameBuffer_vert;

	ShaderProgram* m_SSAOProgram;
	ShaderProgram* m_SSAOViewSpaceZProgram;
	ShaderProgram* m_GaussianProgram_horiz;
	ShaderProgram* m_GaussianProgram_vert;
};

#endif