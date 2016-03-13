#include "Rendering/DrawBloomPass.h"

DrawBloomPass::DrawBloomPass(IRenderer* renderer, ConfigFile* config)
	: m_Renderer(renderer)
	, m_Config(config)
{
	InitializeTextures();

	ChangeQuality(m_Config->Get<int>("GLOW.Quality", 2));
}

DrawBloomPass::~DrawBloomPass() {
	CommonFunctions::DeleteTexture(&m_GaussianTexture_horiz);
	CommonFunctions::DeleteTexture(&m_GaussianTexture_vert);
    CommonFunctions::DeleteTexture(&m_FinalGaussianTexture);
}

void DrawBloomPass::InitializeTextures()
{
    m_BlackTexture = CommonFunctions::TryLoadResource<Texture, false>("Textures/Core/Black.png");
}

void DrawBloomPass::ChangeQuality(int quality)
{
	if (m_Quality == quality) {
		return;
	}
	m_Quality = quality;

	m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.mesh");

	if (m_Quality == 0) {
		CommonFunctions::DeleteTexture(&m_GaussianTexture_horiz);
        CommonFunctions::DeleteTexture(&m_GaussianTexture_vert);
        CommonFunctions::DeleteTexture(&m_FinalGaussianTexture);
		return;
	}
	InitializeTextures();

	InitializeBuffers();
	InitializeShaderPrograms();
	std::string qStr = std::to_string(m_Quality);
	m_Iterations = m_Config->Get<float>("GLOW" + qStr + ".NumIterations", 0);
}

void DrawBloomPass::InitializeShaderPrograms()
{
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

    m_GaussianCombineProgram = ResourceManager::Load<ShaderProgram>("#GaussianCombineProgram");
    if (m_GaussianCombineProgram->GetHandle() == 0) {
        m_GaussianCombineProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/CombineGaussianTexture.vert.glsl")));
        m_GaussianCombineProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/CombineGaussianTexture.frag.glsl")));
        m_GaussianCombineProgram->Compile();
        m_GaussianCombineProgram->BindFragDataLocation(0, "fragmentColor");
        m_GaussianCombineProgram->Link();
    }
}

void DrawBloomPass::InitializeBuffers()
{
    CommonFunctions::GenerateMipMapTexture(
        &m_GaussianTexture_horiz, GL_CLAMP_TO_BORDER, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height),
		GL_RGB8, GL_RGB, GL_FLOAT, m_BloomLod);
    CommonFunctions::GenerateMipMapTexture(
        &m_GaussianTexture_vert, GL_CLAMP_TO_BORDER, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height),
        GL_RGB8, GL_RGB, GL_FLOAT, m_BloomLod);
    CommonFunctions::GenerateTexture(&m_FinalGaussianTexture, GL_CLAMP_TO_BORDER, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);

    if (m_GaussianCombineBuffer.GetHandle() == 0) {
        m_GaussianCombineBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_FinalGaussianTexture, GL_COLOR_ATTACHMENT0)));
    }
    m_GaussianCombineBuffer.Generate();

    if(m_GaussianFrameBuffer_horiz == nullptr) {
        m_GaussianFrameBuffer_horiz = new FrameBuffer[m_BloomLod];
    }
    if (m_GaussianFrameBuffer_vert == nullptr) {
        m_GaussianFrameBuffer_vert = new FrameBuffer[m_BloomLod];
    }

    for (int i = 0; i < m_BloomLod; i++) {
        if(m_GaussianFrameBuffer_horiz[i].GetHandle() == 0) {
            m_GaussianFrameBuffer_horiz[i].AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_GaussianTexture_horiz, GL_COLOR_ATTACHMENT0, i)));
        }
        m_GaussianFrameBuffer_horiz[i].Generate();

        if (m_GaussianFrameBuffer_vert[i].GetHandle() == 0) {
            m_GaussianFrameBuffer_vert[i].AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_GaussianTexture_vert, GL_COLOR_ATTACHMENT0, i)));
        }
        m_GaussianFrameBuffer_vert[i].Generate();
    }
}


void DrawBloomPass::ClearBuffer()
{
	if (m_Quality == 0) {
		return;
	}
    GLERROR("PRE");
    for (int i = 0; i < m_BloomLod; i++) {
        m_GaussianFrameBuffer_horiz[i].Bind();
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        m_GaussianFrameBuffer_horiz[i].Unbind();
        m_GaussianFrameBuffer_vert[i].Bind();
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        m_GaussianFrameBuffer_vert[i].Unbind();

    }
    m_GaussianCombineBuffer.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_GaussianCombineBuffer.Unbind();
    GLERROR("END");
}

void DrawBloomPass::Draw(GLuint texture)
{
    if (m_Quality == 0) {
        return;
    }

    for (int i = 0; i < m_BloomLod; i++) {
        GaussianLodPass(i, texture);
    }
    CombineGaussianBlur();
}


void DrawBloomPass::OnWindowResize()
{
	if (m_Quality == 0) {
		return;
	}
	InitializeBuffers();
}

void DrawBloomPass::GaussianLodPass(GLuint mipMap, GLuint texture)
{
    GLERROR("DrawBloomPass::Draw: Pre");
    glViewport(0, 0, m_Renderer->GetViewportSize().Width/(glm::pow(2, mipMap)), m_Renderer->GetViewportSize().Height/(glm::pow(2, mipMap)));
    DrawBloomPassState state;

    GLuint shaderHandle_horiz = m_GaussianProgram_horiz->GetHandle();
    GLuint shaderHandle_vert = m_GaussianProgram_vert->GetHandle();

    m_GaussianProgram_vert->Bind();
    glUniform1i(glGetUniformLocation(shaderHandle_vert, "Lod"), mipMap);
    m_GaussianProgram_horiz->Bind();
    glUniform1i(glGetUniformLocation(shaderHandle_horiz, "Lod"), mipMap);


    //Horizontal pass, first use the given texture then save it to the horizontal framebuffer.
    m_GaussianFrameBuffer_horiz[mipMap].Bind();


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
    //Iterate some times to make it more gaussian.
    for (int i = 1; i < m_Iterations; i++) {
        //Vertical pass
        m_GaussianFrameBuffer_vert[mipMap].Bind();
        m_GaussianProgram_vert->Bind();


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_horiz);

        glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
            , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
        //horizontal pass
        m_GaussianFrameBuffer_vert[mipMap].Unbind();

        m_GaussianFrameBuffer_horiz[mipMap].Bind();
        m_GaussianProgram_horiz->Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_vert);

        glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
            , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
        m_GaussianFrameBuffer_horiz[mipMap].Unbind();
    }

    //final vertical gaussian after the iterations are done

    m_GaussianFrameBuffer_vert[mipMap].Bind();
    m_GaussianProgram_vert->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_horiz);

    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);

    GLERROR("DrawBloomPass::Draw: END");
    glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    m_GaussianFrameBuffer_vert[mipMap].Unbind();

}

void DrawBloomPass::CombineGaussianBlur()
{
    m_GaussianCombineBuffer.Bind();
    m_GaussianCombineProgram->Bind();
    glUniform1i(glGetUniformLocation(m_GaussianCombineProgram->GetHandle(), "MaxMipMap"), m_BloomLod);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_vert);
    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
}
