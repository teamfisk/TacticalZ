#include "Rendering/BlurHUD.h"

BlurHUD::BlurHUD(IRenderer* renderer)
    : m_Renderer(renderer)
{
    InitializeShaderPrograms();
    InitializeBuffers();
	InitializeTextures();
}

void BlurHUD::InitializeTextures()
{
    m_BlackTexture = CommonFunctions::LoadTexture("Textures/Core/Black.png", false);
    m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.mesh");
}

void BlurHUD::InitializeShaderPrograms()
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
    m_FillDepthStencilProgram = ResourceManager::Load<ShaderProgram>("#FillDepthStencilProgram");
    if (m_FillDepthStencilProgram->GetHandle() == 0) {
        m_FillDepthStencilProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/FillDepthBuffer.vert.glsl")));
        m_FillDepthStencilProgram->Compile();
        m_FillDepthStencilProgram->Link();
    }
    m_CombineTexturesProgram = ResourceManager::Load<ShaderProgram>("#CombineTexturesProgram");
    if (m_CombineTexturesProgram->GetHandle() == 0) {
        m_CombineTexturesProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/CombineTexture.vert.glsl")));
        m_CombineTexturesProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/CombineTexture.frag.glsl")));
        m_CombineTexturesProgram->Compile();
        m_CombineTexturesProgram->BindFragDataLocation(0, "sceneColor");
        m_CombineTexturesProgram->BindFragDataLocation(1, "bloomColor");
        m_CombineTexturesProgram->Link();
    }
    GLERROR("Creating DepthFill program");
}

void BlurHUD::InitializeBuffers()
{
    glm::vec2 res = glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    glm::vec2 res2 = glm::vec2(m_Renderer->GetViewportSize().Width/m_BlurQuality, m_Renderer->GetViewportSize().Height/m_BlurQuality);


    CommonFunctions::GenerateTexture(&m_DepthStencil_horiz, GL_CLAMP_TO_BORDER, GL_NEAREST,
        res2, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    CommonFunctions::GenerateTexture(&m_GaussianTexture_horiz, GL_CLAMP_TO_EDGE, GL_NEAREST, 
        res2, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	if (m_GaussianFrameBuffer_horiz.GetHandle() == 0) {
        m_GaussianFrameBuffer_horiz.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_DepthStencil_horiz, GL_DEPTH_STENCIL_ATTACHMENT)));
		m_GaussianFrameBuffer_horiz.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_GaussianTexture_horiz, GL_COLOR_ATTACHMENT0)));
	}
	m_GaussianFrameBuffer_horiz.Generate();

    CommonFunctions::GenerateTexture(&m_DepthStencil_vert, GL_CLAMP_TO_BORDER, GL_NEAREST,
        res2, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    CommonFunctions::GenerateTexture(&m_GaussianTexture_vert, GL_CLAMP_TO_EDGE, GL_NEAREST,
        res2, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	if (m_GaussianFrameBuffer_vert.GetHandle() == 0) {
        m_GaussianFrameBuffer_vert.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_DepthStencil_vert, GL_DEPTH_STENCIL_ATTACHMENT)));
		m_GaussianFrameBuffer_vert.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_GaussianTexture_vert, GL_COLOR_ATTACHMENT0)));
	}
	m_GaussianFrameBuffer_vert.Generate();

    CommonFunctions::GenerateTexture(&m_CombinedTexture, GL_CLAMP_TO_BORDER, GL_NEAREST,
        res, GL_RGB16F, GL_RGB, GL_FLOAT);

    if (m_CombinedTextureBuffer.GetHandle() == 0) {
        m_CombinedTextureBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_CombinedTexture, GL_COLOR_ATTACHMENT0)));
    }
    m_CombinedTextureBuffer.Generate();
}


void BlurHUD::ClearBuffer()
{
    GLERROR("PRE");
    m_GaussianFrameBuffer_horiz.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClearStencil(0x00);
    glStencilMask(~0);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    m_GaussianFrameBuffer_horiz.Unbind();
    m_GaussianFrameBuffer_vert.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClearStencil(0x00);
    glStencilMask(~0);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    m_GaussianFrameBuffer_vert.Unbind();

    m_CombinedTextureBuffer.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_CombinedTextureBuffer.Unbind();
    GLERROR("END");
}

//Returns the finished blurred texture
GLuint BlurHUD::Draw(GLuint texture, RenderScene& scene)
{
    GLERROR("DrawBloomPass::Draw: Pre");

    FillStencil(scene);

    RenderState state;
    state.Disable(GL_BLEND);
    state.Disable(GL_DEPTH_TEST);
    state.Disable(GL_CULL_FACE);

    GLuint shaderHandle_horiz = m_GaussianProgram_horiz->GetHandle();
    GLuint shaderHandle_vert = m_GaussianProgram_vert->GetHandle();



    //Horizontal pass, first use the given texture then save it to the horizontal framebuffer.
    m_GaussianFrameBuffer_horiz.Bind();
    state.Enable(GL_STENCIL_TEST);
    state.StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    state.StencilFunc(GL_EQUAL, 1, 0xFF);
    state.StencilMask(0x00);
    state.DepthMask(GL_FALSE);
    state.Enable(GL_SCISSOR_TEST);
    glViewport(0, 0, m_Renderer->GetViewportSize().Width/m_BlurQuality, m_Renderer->GetViewportSize().Height/m_BlurQuality);
    glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    
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
        state.Enable(GL_STENCIL_TEST);
        state.StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        state.StencilFunc(GL_EQUAL, 1, 0xFF);
        state.StencilMask(0x00);
        state.DepthMask(GL_FALSE);
        glViewport(0, 0, m_Renderer->GetViewportSize().Width/m_BlurQuality, m_Renderer->GetViewportSize().Height/m_BlurQuality);
        glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
        m_GaussianProgram_vert->Bind();

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_horiz);

        glBindVertexArray(m_ScreenQuad->VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
        glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
            , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
        //horizontal pass
		m_GaussianFrameBuffer_vert.Unbind();

        m_GaussianFrameBuffer_horiz.Bind();
        state.Enable(GL_STENCIL_TEST);
        state.StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        state.StencilFunc(GL_EQUAL, 1, 0xFF);
        state.StencilMask(0x00);
        state.DepthMask(GL_FALSE);
        glViewport(0, 0, m_Renderer->GetViewportSize().Width/m_BlurQuality, m_Renderer->GetViewportSize().Height/m_BlurQuality);
        glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
        m_GaussianProgram_horiz->Bind();

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_vert);

        glBindVertexArray(m_ScreenQuad->VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
        glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
            , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);
		m_GaussianFrameBuffer_horiz.Unbind();
    }

    //final vertical gaussian after the iterations are done

    m_GaussianFrameBuffer_vert.Bind();
    state.Enable(GL_STENCIL_TEST);
    state.StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    state.StencilFunc(GL_EQUAL, 1, 0xFF);
    state.StencilMask(0x00);
    state.DepthMask(GL_FALSE);
    glViewport(0, 0, m_Renderer->GetViewportSize().Width/m_BlurQuality, m_Renderer->GetViewportSize().Height/m_BlurQuality);
    glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    m_GaussianProgram_vert->Bind();

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_horiz);
    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);

    GLERROR("DrawBloomPass::Draw: END");
    glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);


    return m_GaussianTexture_vert;
}


void BlurHUD::OnWindowResize()
{
    CommonFunctions::GenerateTexture(&m_GaussianTexture_vert, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width/m_BlurQuality, m_Renderer->GetViewportSize().Height/m_BlurQuality), GL_RGB16F, GL_RGB, GL_FLOAT);
    m_GaussianFrameBuffer_vert.Generate();
    CommonFunctions::GenerateTexture(&m_GaussianTexture_horiz, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width/m_BlurQuality, m_Renderer->GetViewportSize().Height/m_BlurQuality), GL_RGB16F, GL_RGB, GL_FLOAT);
    m_GaussianFrameBuffer_horiz.Generate();
}

void BlurHUD::FillStencil(RenderScene& scene)
{
    RenderState state;

    state.BindFramebuffer(m_GaussianFrameBuffer_horiz.GetHandle());
    state.Disable(GL_DEPTH_TEST);
    state.Enable(GL_CULL_FACE);
    state.Enable(GL_STENCIL_TEST);
    state.StencilFunc(GL_ALWAYS, 1, 0xFF);
    state.StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    state.StencilMask(0xFF);
    glViewport(0, 0, m_Renderer->GetViewportSize().Width/m_BlurQuality, m_Renderer->GetViewportSize().Height/m_BlurQuality);

    m_FillDepthStencilProgram->Bind();

    GLuint shaderHandle = m_FillDepthStencilProgram->GetHandle();

    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));

    for (auto& job : scene.Jobs.SpriteJob) {
        auto spriteJob = std::dynamic_pointer_cast<SpriteJob>(job);
        if (!spriteJob) {
            continue;
        }
        if (!spriteJob->BlurBackground) {
            continue;
        }
        glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(spriteJob->Matrix));

        glBindVertexArray(spriteJob->Model->VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteJob->Model->ElementBuffer);
        glDrawElements(GL_TRIANGLES, spriteJob->EndIndex - spriteJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(spriteJob->StartIndex*sizeof(unsigned int)));
    }

    state.BindFramebuffer(m_GaussianFrameBuffer_vert.GetHandle());
    for (auto& job : scene.Jobs.SpriteJob) {
        auto spriteJob = std::dynamic_pointer_cast<SpriteJob>(job);
        if (!spriteJob) {
            continue;
        }
        if(!spriteJob->BlurBackground) {
            continue;
        }
        glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(spriteJob->Matrix));

        glBindVertexArray(spriteJob->Model->VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteJob->Model->ElementBuffer);
        glDrawElements(GL_TRIANGLES, spriteJob->EndIndex - spriteJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(spriteJob->StartIndex*sizeof(unsigned int)));
    }
    glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
}

//Texture 1 will be used if texture 2 is black at that texel, else texture 2 is used.
GLuint BlurHUD::CombineTextures(GLuint texture1, GLuint texture2)
{
    //RenderState state;
    //state.BindFramebuffer(m_CombinedTextureBuffer.GetHandle());
    //state.Disable(GL_DEPTH_TEST);
    //state.Disable(GL_STENCIL_TEST);

    m_CombineTexturesProgram->Bind();

    glActiveTexture(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].material->EndIndex - m_ScreenQuad->MaterialGroups()[0].material->StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].material->StartIndex);

    return m_CombinedTexture;
}
