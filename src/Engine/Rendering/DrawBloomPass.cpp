#include "Rendering/DrawBloomPass.h"

DrawBloomPass::DrawBloomPass(IRenderer* renderer, ConfigFile* config)
	: m_Renderer(renderer)
	, m_Config(config)
{
	InitializeTextures();

	ChangeQuality(m_Config->Get<int>("GLOW.Quality", 2));
}

void DrawBloomPass::ChangeQuality(int quality)
{
	if (m_Quality == quality) {
		return;
	}
	m_Quality = quality;

	m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.mesh");

	if (m_Quality == 0) {
		glDeleteTextures(1, &m_GaussianTexture_horiz);
		glDeleteTextures(1, &m_GaussianTexture_vert);
		return;
	}

	std::string qStr = std::to_string(m_Quality);
	m_Iterations = m_Config->Get<float>("GLOW" + qStr + ".NumIterations", 0);

	InitializeBuffers();
	InitializeShaderPrograms();
}

void DrawBloomPass::InitializeTextures()
{
    m_BlackTexture = CommonFunctions::LoadTexture("Textures/Core/Black.png", false);
}

void DrawBloomPass::InitializeShaderPrograms()
{
    m_GaussianProgram_horiz = ResourceManager::Load<ShaderProgram>("##GaussianProgramHoriz");
	if (m_GaussianProgram_horiz->GetHandle() == 0) {
		m_GaussianProgram_horiz->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Gaussian_horiz.vert.glsl")));
		m_GaussianProgram_horiz->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Gaussian_horiz.frag.glsl")));
		m_GaussianProgram_horiz->Compile();
		m_GaussianProgram_horiz->Link();
	}

	m_GaussianProgram_vert = ResourceManager::Load<ShaderProgram>("##GaussianProgramVert");
	if (m_GaussianProgram_vert->GetHandle() == 0) {
		m_GaussianProgram_vert->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Gaussian_vert.vert.glsl")));
		m_GaussianProgram_vert->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Gaussian_vert.frag.glsl")));
		m_GaussianProgram_vert->Compile();
		m_GaussianProgram_vert->Link();
	}
}


void DrawBloomPass::InitializeBuffers()
{
    GenerateTexture(&m_GaussianTexture_horiz, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);

	if (m_GaussianFrameBuffer_horiz.GetHandle() == 0) {
		m_GaussianFrameBuffer_horiz.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_GaussianTexture_horiz, GL_COLOR_ATTACHMENT0)));
	}
    m_GaussianFrameBuffer_horiz.Generate();

    GenerateTexture(&m_GaussianTexture_vert, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
	if (m_GaussianFrameBuffer_vert.GetHandle() == 0) {
		m_GaussianFrameBuffer_vert.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_GaussianTexture_vert, GL_COLOR_ATTACHMENT0)));
	}
    m_GaussianFrameBuffer_vert.Generate();
}


void DrawBloomPass::ClearBuffer()
{
	if (m_Quality == 0) {
		return;
	}
    GLERROR("PRE");
    m_GaussianFrameBuffer_horiz.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_GaussianFrameBuffer_horiz.Unbind();
    m_GaussianFrameBuffer_vert.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_GaussianFrameBuffer_vert.Unbind();
    GLERROR("END");
}

void DrawBloomPass::Draw(GLuint texture)
{
	if (m_Quality == 0) {
		return;
	}
    GLERROR("DrawBloomPass::Draw: Pre");

    DrawBloomPassState state;

    GLuint shaderHandle_horiz = m_GaussianProgram_horiz->GetHandle();
    GLuint shaderHandle_vert = m_GaussianProgram_vert->GetHandle();


    //Horizontal pass, first use the given texture then save it to the horizontal framebuffer.
    m_GaussianFrameBuffer_horiz.Bind();
    m_GaussianProgram_horiz->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
    //Iterate some times to make it more gaussian.
    for (int i = 1; i < m_Iterations; i++) {
        //Vertical pass
        m_GaussianFrameBuffer_vert.Bind();
        m_GaussianProgram_vert->Bind();

        glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_horiz);

        glBindVertexArray(m_ScreenQuad->VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
        glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
            , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
        //horizontal pass

        m_GaussianFrameBuffer_horiz.Bind();
        m_GaussianProgram_horiz->Bind();

        glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_vert);

        glBindVertexArray(m_ScreenQuad->VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
        glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
            , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
    }

    //final vertical gaussian after the iterations are done

    m_GaussianFrameBuffer_vert.Bind();
    m_GaussianProgram_vert->Bind();

    glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_horiz);
    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);

    GLERROR("DrawBloomPass::Draw: END");
}


void DrawBloomPass::OnWindowResize()
{
	if (m_Quality == 0) {
		return;
	}
    GenerateTexture(&m_GaussianTexture_vert, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    m_GaussianFrameBuffer_vert.Generate();
    GenerateTexture(&m_GaussianTexture_horiz, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    m_GaussianFrameBuffer_horiz.Generate();
}

void DrawBloomPass::GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const
{
	glDeleteTextures(1, texture);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, dimensions.x, dimensions.y, 0, format, type, nullptr);//TODO: Renderer: Fix the precision and Resolution
    GLERROR("Texture initialization failed");
}
