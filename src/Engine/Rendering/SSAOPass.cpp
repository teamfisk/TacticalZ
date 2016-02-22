#include "Rendering/SSAOPass.h"

SSAOPass::SSAOPass(IRenderer* renderer)
{
	m_Renderer = renderer;

	m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.mesh");

	InitializeBuffer();
	InitializeShaderProgram();
	Setting(0.1f, 0.012f, 1.0f, 1.0f, 13, 7);

	m_DrawBloomPass = new DrawBloomPass(renderer);
}

void SSAOPass::InitializeShaderProgram()
{
	m_SSAOProgram = ResourceManager::Load<ShaderProgram>("##SSAOProgram");
	m_SSAOProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/SSAO.vert.glsl")));
	m_SSAOProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/SSAO.frag.glsl")));
	m_SSAOProgram->Compile();
	m_SSAOProgram->Link();

	m_SSAOViewSpaceZProgram = ResourceManager::Load<ShaderProgram>("##SSAOViewSpaceZProgram");
	m_SSAOViewSpaceZProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/SSAO.vert.glsl")));
	m_SSAOViewSpaceZProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/SSAOViewSpaceZ.frag.glsl")));
	m_SSAOViewSpaceZProgram->Compile();
	m_SSAOViewSpaceZProgram->Link();
}


void SSAOPass::InitializeBuffer()
{
	GenerateTexture(&m_SSAOTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_R8, GL_RED, GL_FLOAT);

	m_SSAOFramBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_SSAOTexture, GL_COLOR_ATTACHMENT0)));
	m_SSAOFramBuffer.Generate();

	GenerateTexture(&m_SSAOViewSpaceZTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_R32F, GL_RED, GL_FLOAT);

	m_SSAOViewSpaceZFramBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_SSAOViewSpaceZTexture, GL_COLOR_ATTACHMENT0)));
	m_SSAOViewSpaceZFramBuffer.Generate();
}

void SSAOPass::ClearBuffer()
{
	m_SSAOFramBuffer.Bind();
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_SSAOFramBuffer.Unbind();

	m_SSAOViewSpaceZFramBuffer.Bind();
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_SSAOViewSpaceZFramBuffer.Unbind();
}

void SSAOPass::Setting(float radius, float bias, float contrast, float intensityScale, int numOfSamples, int NumOfTurns) {
	m_Radius = radius;
	m_Bias = bias;
	m_Contrast = contrast;
	m_IntensityScale = intensityScale;
	m_NumOfSamples = numOfSamples;
	m_NumOfTurns = NumOfTurns;
}

void SSAOPass::GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, dimensions.x, dimensions.y, 0, format, type, nullptr);
	GLERROR("Texture initialization failed");
}

void SSAOPass::Draw(GLuint depthBuffer, Camera* camera)
{
	SSAOPassState state;
	GLuint viewSpaceZPShaderHandle = m_SSAOViewSpaceZProgram->GetHandle();
	GLuint SSAOShaderHandle = m_SSAOProgram->GetHandle();

	m_SSAOViewSpaceZFramBuffer.Bind();
	m_SSAOViewSpaceZProgram->Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthBuffer);
	glm::vec3 clipInfo = glm::vec3(
		(camera->NearClip() * camera->FarClip()),
		(camera->NearClip() - camera->FarClip()),
		(camera->FarClip())
		);
	/*glm::vec3 clipInfo = glm::vec3(
		(camera->NearClip()),
		(-1.0f),
		(+1.0f)
		);*/
	glUniform3fv(glGetUniformLocation(viewSpaceZPShaderHandle, "ClipInfo"), 1, glm::value_ptr(clipInfo));

	glBindVertexArray(m_ScreenQuad->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
	glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex + 1
		, GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
	
	glm::vec4 projInfo = glm::vec4(
		((1.0 - camera->ProjectionMatrix()[0][2]) / camera->ProjectionMatrix()[0][0]),
		(-2.0 / (m_Renderer->GetViewportSize().Width * camera->ProjectionMatrix()[0][0])),
		((1.0 + camera->ProjectionMatrix()[1][2]) / camera->ProjectionMatrix()[1][1]),
		(-2.0 / (m_Renderer->GetViewportSize().Height * camera->ProjectionMatrix()[1][1]))
		);


	m_SSAOFramBuffer.Bind();
	m_SSAOProgram->Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SSAOViewSpaceZTexture);

	// How many pixel there are in a 1m long object 1m away from the camera
	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uProjScale"), m_Renderer->GetViewportSize().Height / (-2.0f * glm::tan(camera->FOV() * 0.5f)));

	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uRadius"), m_Radius);
	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uBias"), m_Bias);
	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uContrast"), m_Contrast);
	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uIntensityScale"), m_IntensityScale);
	glUniform1i(glGetUniformLocation(SSAOShaderHandle, "uNumOfSamples"), m_NumOfSamples);
	glUniform1i(glGetUniformLocation(SSAOShaderHandle, "uNumOfTurns"), m_NumOfTurns);;

	glUniform4fv(glGetUniformLocation(SSAOShaderHandle, "uProjInfo"), 1, glm::value_ptr(projInfo));
	glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex + 1
		, GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);

	m_DrawBloomPass->ClearBuffer();
	m_DrawBloomPass->Draw(m_SSAOTexture);
}

void ComputeAO(GLuint depthBuffer, Camera* camera) {

}