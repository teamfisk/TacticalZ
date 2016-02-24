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
	SSAOPass(IRenderer* rendere);
	~SSAOPass() { };

	void Draw(GLuint depthBuffer, Camera* camera);
	void Setting(float radius, float bias, float contrast, float intensityScale, int numOfSamples, int NumOfTurns);
	void ClearBuffer();

	//Return the SSAO of the texture sent to Draw
	GLuint SSAOTexture() const { return m_DrawBloomPass->GaussianTexture(); }

private:
	void InitializeTexture();
	void InitializeFrameBuffer();
	void InitializeShaderProgram();
	void InitializeBuffer();

	void ComputeAO(GLuint depthBuffer, Camera* camera);
	//void blurHorizontal(GLuint depthBuffer);
	//void blurVertical(GLuint depthBuffer);

	Model* m_ScreenQuad;

	const IRenderer* m_Renderer;
	static const GLint MAX_MIP_LEVEL = 5;
	float m_Radius;
	float m_Bias;
	float m_Contrast;
	float m_IntensityScale;
	int m_NumOfSamples;
	int m_NumOfTurns;

	GLuint m_SSAOTexture;
	FrameBuffer m_SSAOFramBuffer;

	GLuint m_SSAOViewSpaceZTexture;
	FrameBuffer m_SSAOViewSpaceZFramBuffer;

	ShaderProgram* m_SSAOProgram;
	ShaderProgram* m_SSAOViewSpaceZProgram;

	DrawBloomPass* m_DrawBloomPass;
};

#endif