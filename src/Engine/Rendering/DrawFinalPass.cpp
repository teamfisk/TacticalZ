#include "Rendering/DrawFinalPass.h"

DrawFinalPass::DrawFinalPass(IRenderer* renderer, LightCullingPass* lightCullingPass)
{
    m_Renderer = renderer;
    m_LightCullingPass = lightCullingPass;
    InitializeTextures();
    InitializeShaderPrograms();
    InitializeFrameBuffers();
}

void DrawFinalPass::InitializeTextures()
{
    m_WhiteTexture = ResourceManager::Load<Texture>("Textures/Core/Blank.png");
    m_BlackTexture = ResourceManager::Load<Texture>("Textures/Core/Black.png");
    TEMP_glowTestTexture = ResourceManager::Load<Texture>("Textures/Core/UnitRaptor_glow.png");
}

void DrawFinalPass::InitializeFrameBuffers()
{
    glGenRenderbuffers(1, &m_DepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

    GenerateTexture(&m_SceneTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->Resolution().Width, m_Renderer->Resolution().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->Resolution().Width, m_Renderer->Resolution().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->Resolution().Width, m_Renderer->Resolution().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    GenerateMipMapTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, glm::vec2(m_Renderer->Resolution().Width, m_Renderer->Resolution().Height), GL_RGB16F, GL_FLOAT, 4);
    
    m_BloomFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new RenderBuffer(&m_DepthBuffer, GL_DEPTH_ATTACHMENT)));
    m_BloomFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_SceneTexture, GL_COLOR_ATTACHMENT0)));
    m_BloomFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_BloomTexture, GL_COLOR_ATTACHMENT1)));
    m_BloomFrameBuffer.Generate();

}

void DrawFinalPass::InitializeShaderPrograms()
{
    m_ForwardPlusProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusProgram");
    m_ForwardPlusProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
    m_ForwardPlusProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlus.frag.glsl")));
    m_ForwardPlusProgram->Compile();
    m_ForwardPlusProgram->Link();
}

void DrawFinalPass::Draw(RenderScene& scene)
{
    GLERROR("DrawFinalPass::Draw: Pre");

    m_BloomFrameBuffer.Bind(); //in i state

    DrawFinalPassState state;
    m_ForwardPlusProgram->Bind();
    GLuint shaderHandle = m_ForwardPlusProgram->GetHandle();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightCullingPass->LightSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightCullingPass->LightGridSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightCullingPass->LightIndexSSBO());

    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_Renderer->Camera()->ViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_Renderer->Camera()->ProjectionMatrix()));
    glUniform2f(glGetUniformLocation(shaderHandle, "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

    //TODO: Render: Add code for more jobs than modeljobs.
    for (auto &job : scene.ForwardJobs) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
        if(modelJob) {
            //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
            glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(modelJob->Color));

            glActiveTexture(GL_TEXTURE0);
            if(modelJob->DiffuseTexture != nullptr) {
                glBindTexture(GL_TEXTURE_2D, modelJob->DiffuseTexture->m_Texture);
            } else {
                glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
            }
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_BlackTexture->m_Texture);

            /*if(modelJob->GlowMap != nullptr) {
                glBindTexture(GL_TEXTURE_2D, modelJob->GlowMap->m_Texture);
            } else {
                glBindTexture(GL_TEXTURE_2D, m_BlackTexture->m_Texture);
            }*/

            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElementsBaseVertex(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, 0, modelJob->StartIndex);

            continue;
        }
    }
    m_BloomFrameBuffer.Unbind();
    GLERROR("DrawFinalPass::Draw: END");
}

void DrawFinalPass::GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const
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

void DrawFinalPass::GenerateMipMapTexture(GLuint* texture, GLenum wrapping, glm::vec2 dimensions, GLint format, GLenum type, GLint numMipMaps) const
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexStorage2D(GL_TEXTURE_2D, numMipMaps, GL_RGBA8, dimensions.x, dimensions.y);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dimensions.x, dimensions.y, format, type, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    GLERROR("MipMap Texture initialization failed");
}
