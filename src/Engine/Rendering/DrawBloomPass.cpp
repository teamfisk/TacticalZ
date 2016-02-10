#include "Rendering/DrawBloomPass.h"

DrawBloomPass::DrawBloomPass(IRenderer* renderer)
{
    m_Renderer = renderer;

    m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.mesh");

    m_ScreenSizes[0] = 1920.f;
    m_ScreenSizes[1] = 1053.f;
    m_ScreenSizes[2] = 959.f;
    m_ScreenSizes[3] = 525.f;
    m_ScreenSizes[4] = 479.f;
    m_ScreenSizes[5] = 262.f;
    m_ScreenSizes[6] = 239.f;
    m_ScreenSizes[7] = 130.f;
    m_ScreenSizes[8] = 119.f;
    m_ScreenSizes[9] = 64.f;


    InitializeTextures();
    InitializeBuffers();
    InitializeShaderPrograms();
}

void DrawBloomPass::InitializeTextures()
{
    m_WhiteTexture = ResourceManager::Load<Texture>("Textures/Core/White.png");
}

void DrawBloomPass::InitializeShaderPrograms()
{
    m_GaussianProgram_horiz = ResourceManager::Load<ShaderProgram>("#GaussianProgramHoriz");
    m_GaussianProgram_horiz->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Gaussian_horiz.vert.glsl")));
    m_GaussianProgram_horiz->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Gaussian_horiz.frag.glsl")));
    m_GaussianProgram_horiz->Compile();
    m_GaussianProgram_horiz->Link();

    m_GaussianProgram_vert = ResourceManager::Load<ShaderProgram>("#GaussianProgramVert");
    m_GaussianProgram_vert->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Gaussian_vert.vert.glsl")));
    m_GaussianProgram_vert->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Gaussian_vert.frag.glsl")));
    m_GaussianProgram_vert->Compile();
    m_GaussianProgram_vert->Link();

    m_GaussianProgram_both = ResourceManager::Load<ShaderProgram>("#GaussianProgramBoth");
    m_GaussianProgram_both->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Gaussian_both.vert.glsl")));
    m_GaussianProgram_both->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Gaussian_both.frag.glsl")));
    m_GaussianProgram_both->Compile();
    m_GaussianProgram_both->Link();

    m_CombineProgram = ResourceManager::Load<ShaderProgram>("#CombineProgram");
    m_CombineProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/TextureCombine.vert.glsl")));
    m_CombineProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/TextureCombine.frag.glsl")));
    m_CombineProgram->Compile();
    m_CombineProgram->Link();

    //Fixa nya combine shadern
}


void DrawBloomPass::InitializeBuffers()
{
    GenerateTexture(&m_GaussianTexture_horiz, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_HALF_FLOAT);

    m_GaussianFrameBuffer_horiz.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_GaussianTexture_horiz, GL_COLOR_ATTACHMENT0)));
    m_GaussianFrameBuffer_horiz.Generate();

    GenerateTexture(&m_GaussianTexture_vert, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_HALF_FLOAT);

    m_GaussianFrameBuffer_vert.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_GaussianTexture_vert, GL_COLOR_ATTACHMENT0)));
    m_GaussianFrameBuffer_vert.Generate();

    GenerateTexture(&m_FinalBlurTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);

    m_BlurredFinalTexture.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_FinalBlurTexture, GL_COLOR_ATTACHMENT0)));
    m_BlurredFinalTexture.Generate();

}


void DrawBloomPass::ClearBuffer()
{
    m_GaussianFrameBuffer_horiz.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_GaussianFrameBuffer_horiz.Unbind();
    m_GaussianFrameBuffer_vert.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_GaussianFrameBuffer_vert.Unbind();
}

void DrawBloomPass::Draw(GLuint texture)
{

    
    ImGui::Combo("DebugBloom", &m_DebugTextureToDraw, "ALL\0First\0SEcond\0Third\0Fourth\0Fifth");
    ImGui::SliderInt("Iterations", &m_Iterations, 0, 10);

    if (m_DebugTextureToDraw == 1)
        BlurOneMipMapLevel(&texture, m_Iterations, 1);
    else if (m_DebugTextureToDraw == 2)
        BlurOneMipMapLevel(&texture, m_Iterations, 2);
    else if (m_DebugTextureToDraw == 3)
        BlurOneMipMapLevel(&texture, m_Iterations, 3);
    else if (m_DebugTextureToDraw == 4)
        BlurOneMipMapLevel(&texture, m_Iterations, 4);
    else if (m_DebugTextureToDraw == 0) {
        BlurOneMipMapLevel(&texture, m_Iterations, 1);
        BlurOneMipMapLevel(&texture, m_Iterations, 2);
        BlurOneMipMapLevel(&texture, m_Iterations, 3);
        BlurOneMipMapLevel(&texture, m_Iterations, 4);
    }

}

void DrawBloomPass::BlurOneMipMapLevel(GLuint* texture, GLint iterations, GLint mipMapLevel)
{
    GLERROR("PRE");

    //new code
    DrawBloomPassState state;
    //Loop through and blur texture at mipmap level
    GLuint shaderHandle_horiz = m_GaussianProgram_horiz->GetHandle();
    GLuint shaderHandle_vert = m_GaussianProgram_vert->GetHandle();


    //First horiz pass
    m_GaussianFrameBuffer_horiz.Bind();
    m_GaussianProgram_horiz->Bind();
    GLERROR("Bind");


    glUniform1i(glGetUniformLocation(shaderHandle_horiz, "MipMapLevel"), mipMapLevel);
    //glUniform2fv(glGetUniformLocation(m_GaussianProgram_both->GetHandle(), "ScreenSize"), 5, m_ScreenSizes);
    //glUniform1i(glGetUniformLocation(m_GaussianProgram_both->GetHandle(), "Horizontal"), true);
    GLERROR("1");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *texture);


    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].EndIndex - m_ScreenQuad->MaterialGroups()[0].StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].StartIndex);
    GLERROR("2");


    //Loop through vert, then horiz and save to respective textures X number of times.
    for (int i = 1; i < iterations; i++) {
        //Vertical pass
        m_GaussianFrameBuffer_vert.Bind();
        m_GaussianProgram_vert->Bind();

        //glUniform1i(glGetUniformLocation(m_GaussianProgram_both->GetHandle(), "Horizontal"), false);

        glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_horiz);

        glBindVertexArray(m_ScreenQuad->VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
        glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].EndIndex - m_ScreenQuad->MaterialGroups()[0].StartIndex +1
            , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].StartIndex);
        GLERROR("3");

        //horizontal pass

        m_GaussianFrameBuffer_horiz.Bind();
        m_GaussianProgram_horiz->Bind();

        //glUniform1i(glGetUniformLocation(m_GaussianProgram_both->GetHandle(), "Horizontal"), true);

        glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_vert);

        glBindVertexArray(m_ScreenQuad->VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
        glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].EndIndex - m_ScreenQuad->MaterialGroups()[0].StartIndex +1
            , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].StartIndex);
        GLERROR("4");

    }

    //the final vertical iteration.
    m_GaussianFrameBuffer_vert.Bind();
    m_GaussianProgram_vert->Bind();

    //glUniform1i(glGetUniformLocation(m_GaussianProgram_both->GetHandle(), "Horizontal"), false);

    glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_horiz);

    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].EndIndex - m_ScreenQuad->MaterialGroups()[0].StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].StartIndex);
    GLERROR("5");

    //Combine the the new blurred texture into the final texture
    m_BlurredFinalTexture.Bind();
    m_CombineProgram->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FinalBlurTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_GaussianTexture_vert);

    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].EndIndex - m_ScreenQuad->MaterialGroups()[0].StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].StartIndex);

    GLERROR("END");
}

void DrawBloomPass::GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, dimensions.x, dimensions.y, 0, format, type, nullptr);//TODO: Renderer: Fix the precision and Resolution
    GLERROR("Texture initialization failed");
}


