#include "Rendering/SSAOPass.h"

SSAOPass::SSAOPass(IRenderer* renderer, ConfigFile* config)
	: m_Renderer(renderer)
	, m_Config(config)
{
	m_WhiteTexture = CommonFunctions::TryLoadResource<Texture, false>("Textures/Core/White.png");

	ChangeQuality(m_Config->Get<int>("SSAO.Quality", 0));

}

void SSAOPass::ChangeQuality(int quality)
{
	if (m_Quality == quality) {
		return;
	}

	m_Quality = quality;

	if (m_Quality == 0) {	
		CommonFunctions::DeleteTexture(&m_SSAOTexture);
		CommonFunctions::DeleteTexture(&m_SSAOViewSpaceZTexture);
		CommonFunctions::DeleteTexture(&m_Gaussian_horiz);
		CommonFunctions::DeleteTexture(&m_Gaussian_vert);
		return;
	}

	std::string qStr = std::to_string(m_Quality);
	Setting(
		m_Config->Get<float>("SSAO" + qStr + ".Radius", 0.01),
		m_Config->Get<float>("SSAO" + qStr + ".Bias", 0.012),
		m_Config->Get<float>("SSAO" + qStr + ".Contrast", 1.0),
		m_Config->Get<float>("SSAO" + qStr + ".Intensity", 1.0),
		m_Config->Get<int>("SSAO" + qStr + ".NumSamples", 0),
		m_Config->Get<int>("SSAO" + qStr + ".NumTurns", 0),
		m_Config->Get<int>("SSAO" + qStr + ".NumIterations", 0),
		m_Config->Get<int>("SSAO" + qStr + ".TextureQuality", 4)
		);



	m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.mesh");

	InitializeTexture();
	InitializeBuffer();
	InitializeShaderProgram();


}

void SSAOPass::InitializeShaderProgram()
{
	m_SSAOProgram = ResourceManager::Load<ShaderProgram>("##SSAOProgram");
	if (m_SSAOProgram->GetHandle() == 0) {
		m_SSAOProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/SSAO.vert.glsl")));
		m_SSAOProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/SSAO.frag.glsl")));
		m_SSAOProgram->Compile();
        m_SSAOProgram->BindFragDataLocation(0, "AO");
		m_SSAOProgram->Link();
	}

	m_SSAOViewSpaceZProgram = ResourceManager::Load<ShaderProgram>("##SSAOViewSpaceZProgram");
	if (m_SSAOViewSpaceZProgram->GetHandle() == 0) {
		m_SSAOViewSpaceZProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/SSAO.vert.glsl")));
		m_SSAOViewSpaceZProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/SSAOViewSpaceZ.frag.glsl")));
		m_SSAOViewSpaceZProgram->Compile();
        m_SSAOViewSpaceZProgram->BindFragDataLocation(0, "depthLinear");
		m_SSAOViewSpaceZProgram->Link();
	}

	m_GaussianProgram_horiz = ResourceManager::Load<ShaderProgram>("##GaussianProgramHoriz");
	if (m_GaussianProgram_horiz->GetHandle() == 0) {
		m_GaussianProgram_horiz->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Gaussian_horiz.vert.glsl")));
		m_GaussianProgram_horiz->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Gaussian_horiz.frag.glsl")));
		m_GaussianProgram_horiz->Compile();
        m_GaussianProgram_horiz->BindFragDataLocation(0, "fragmentColor");
		m_GaussianProgram_horiz->Link();
	}

	m_GaussianProgram_vert = ResourceManager::Load<ShaderProgram>("##GaussianProgramVert");
	if (m_GaussianProgram_vert->GetHandle() == 0) {
		m_GaussianProgram_vert->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Gaussian_vert.vert.glsl")));
		m_GaussianProgram_vert->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Gaussian_vert.frag.glsl")));
		m_GaussianProgram_vert->Compile();
        m_GaussianProgram_vert->BindFragDataLocation(0, "fragmentColor");
		m_GaussianProgram_vert->Link();
	}
}

void SSAOPass::InitializeTexture() {
	CommonFunctions::GenerateTexture(&m_SSAOTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width >> m_TextureQuality, m_Renderer->GetViewportSize().Height >> m_TextureQuality), GL_R8, GL_RED, GL_FLOAT);
	CommonFunctions::GenerateTexture(&m_SSAOViewSpaceZTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width >> m_TextureQuality, m_Renderer->GetViewportSize().Height >> m_TextureQuality), GL_R32F, GL_RED, GL_FLOAT);

	CommonFunctions::GenerateTexture(&m_Gaussian_horiz, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width >> m_TextureQuality, m_Renderer->GetViewportSize().Height >> m_TextureQuality), GL_R8, GL_RED, GL_FLOAT);
	CommonFunctions::GenerateTexture(&m_Gaussian_vert, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width >> m_TextureQuality, m_Renderer->GetViewportSize().Height >> m_TextureQuality), GL_R8, GL_RED, GL_FLOAT);
}

void SSAOPass::InitializeBuffer()
{
	if (m_SSAOFramBuffer.GetHandle() == 0) {
		m_SSAOFramBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_SSAOTexture, GL_COLOR_ATTACHMENT0)));
	}
	m_SSAOFramBuffer.Generate();


	if (m_SSAOViewSpaceZFramBuffer.GetHandle() == 0) {
		m_SSAOViewSpaceZFramBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_SSAOViewSpaceZTexture, GL_COLOR_ATTACHMENT0)));
	}
	m_SSAOViewSpaceZFramBuffer.Generate();



	if (m_GaussianFrameBuffer_horiz.GetHandle() == 0) {
		m_GaussianFrameBuffer_horiz.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_Gaussian_horiz, GL_COLOR_ATTACHMENT0)));
	}
	m_GaussianFrameBuffer_horiz.Generate();


	if (m_GaussianFrameBuffer_vert.GetHandle() == 0) {
		m_GaussianFrameBuffer_vert.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_Gaussian_vert, GL_COLOR_ATTACHMENT0)));
	}
	m_GaussianFrameBuffer_vert.Generate();

}

void SSAOPass::ClearBuffer()
{
	if (m_Quality == 0) {
		return;
	}
	m_SSAOFramBuffer.Bind();
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_SSAOFramBuffer.Unbind();

	m_SSAOViewSpaceZFramBuffer.Bind();
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_SSAOViewSpaceZFramBuffer.Unbind();

	m_GaussianFrameBuffer_horiz.Bind();
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_GaussianFrameBuffer_horiz.Unbind();

	m_GaussianFrameBuffer_vert.Bind();
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_GaussianFrameBuffer_vert.Unbind();
}

void SSAOPass::Setting(float radius, float bias, float contrast, float intensityScale, int numOfSamples, int numOfTurns, int iterations, int quality) {
	m_Radius = radius;
	m_Bias = bias;
	m_Contrast = contrast;
	m_IntensityScale = intensityScale;
	m_NumOfSamples = numOfSamples;
	m_NumOfTurns = numOfTurns;
	m_Iterations = iterations;
	m_TextureQuality = quality;
}

void SSAOPass::Draw(GLuint depthBuffer, Camera* camera)
{
	if (m_Quality == 0) {
		return;
	}

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
	glViewport(0, 0, (m_Renderer->GetViewportSize().Width >> m_TextureQuality), (m_Renderer->GetViewportSize().Height >> m_TextureQuality)); //JOHAN TODO: Get this into state
	glUniform3fv(glGetUniformLocation(viewSpaceZPShaderHandle, "ClipInfo"), 1, glm::value_ptr(clipInfo));

	glBindVertexArray(m_ScreenQuad->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
	glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex + 1
		, GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
	
	glm::vec4 projInfo = glm::vec4(
		((1.0 - camera->ProjectionMatrix()[0][2]) / camera->ProjectionMatrix()[0][0]),
		(-2.0 / ((m_Renderer->GetViewportSize().Width >> m_TextureQuality) * camera->ProjectionMatrix()[0][0])),
		((1.0 + camera->ProjectionMatrix()[1][2]) / camera->ProjectionMatrix()[1][1]),
		(-2.0 / ((m_Renderer->GetViewportSize().Height >> m_TextureQuality) * camera->ProjectionMatrix()[1][1]))
		);


	m_SSAOFramBuffer.Bind();
	m_SSAOProgram->Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SSAOViewSpaceZTexture);

	// How many pixel there are in a 1m long object 1m away from the camera
	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uProjScale"), (m_Renderer->GetViewportSize().Height >> m_TextureQuality) / (-2.0f * glm::tan(camera->FOV() * 0.5f)));

	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uRadius"), m_Radius);
	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uBias"), m_Bias);
	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uContrast"), m_Contrast);
	glUniform1f(glGetUniformLocation(SSAOShaderHandle, "uIntensityScale"), m_IntensityScale);
	glUniform1i(glGetUniformLocation(SSAOShaderHandle, "uNumOfSamples"), m_NumOfSamples);
	glUniform1i(glGetUniformLocation(SSAOShaderHandle, "uNumOfTurns"), m_NumOfTurns);

	glUniform4fv(glGetUniformLocation(SSAOShaderHandle, "uProjInfo"), 1, glm::value_ptr(projInfo));
	glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex + 1
		, GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);

	DrawBloomPassState BloomState;
	GLuint shaderHandle_horiz = m_GaussianProgram_horiz->GetHandle();
	GLuint shaderHandle_vert = m_GaussianProgram_vert->GetHandle();

	m_GaussianFrameBuffer_horiz.Bind();
	m_GaussianProgram_horiz->Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SSAOTexture);

	glBindVertexArray(m_ScreenQuad->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
	glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex + 1
		, GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
	//Iterate some times to make it more gaussian.
	for (int i = 1; i < m_Iterations; i++) {
		//Vertical pass
		m_GaussianFrameBuffer_vert.Bind();
		m_GaussianProgram_vert->Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_Gaussian_horiz);

		glBindVertexArray(m_ScreenQuad->VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
		glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex + 1
			, GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
		//horizontal pass
		m_GaussianFrameBuffer_vert.Unbind();

		m_GaussianFrameBuffer_horiz.Bind();
		m_GaussianProgram_horiz->Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_Gaussian_vert);

		glBindVertexArray(m_ScreenQuad->VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
		glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex + 1
			, GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
		m_GaussianFrameBuffer_horiz.Unbind();
	}

	//final vertical gaussian after the iterations are done

	m_GaussianFrameBuffer_vert.Bind();
	m_GaussianProgram_vert->Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Gaussian_horiz);
	glBindVertexArray(m_ScreenQuad->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
	glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex + 1
		, GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);

	m_GaussianFrameBuffer_vert.Unbind();

	glViewport(0, 0, (m_Renderer->GetViewportSize().Width), (m_Renderer->GetViewportSize().Height));
}

void SSAOPass::OnWindowResize() {
	if (m_Quality == 0) {
		return;
	}

	InitializeTexture();
	InitializeBuffer();
}