#include "Rendering/DrawScenePass.h"

DrawScenePass::DrawScenePass(IRenderer* renderer)
{
    m_Renderer = renderer;
    InitializeTextures();
    InitializeShaderPrograms();
}

void DrawScenePass::InitializeTextures()
{
    m_WhiteTexture = ResourceManager::Load<Texture>("Textures/Core/Blank.png");
}

void DrawScenePass::InitializeShaderPrograms()
{
    //Gör så att shaders är en resource, tex som texture classen. Konstruktorn måste vara privat.
    m_BasicForwardProgram = ResourceManager::Load<ShaderProgram>("#BasicForwardProgram");
    m_BasicForwardProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/BasicForward.vert.glsl")));
    m_BasicForwardProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/BasicForward.frag.glsl")));
    m_BasicForwardProgram->Compile();
    m_BasicForwardProgram->Link();

    m_CoolDeathAnimProgram = ResourceManager::Load<ShaderProgram>("#CoolDeathAnimProgram");
    m_CoolDeathAnimProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/CoolDeathAnim.vert.glsl")));
    m_CoolDeathAnimProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/CoolDeathAnim.geom.glsl")));
    m_CoolDeathAnimProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/CoolDeathAnim.frag.glsl")));
    m_CoolDeathAnimProgram->Compile();
    m_CoolDeathAnimProgram->Link();
    
}

void DrawScenePass::Draw(RenderQueueCollection& rq)
{

    if (TimeDeath > 2.f) {
        TimeDeath = 0.f;
    }

    Origin = glm::vec3(0.f, 0.0f, 0.f);
    TimeDeath = TimeDeath + 0.01f;
    EndDeath = EndDeath + 0.05f;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLERROR("Renderer::Draw PickingPass");

    DrawScenePassState state;


    //TODO: Render: Add code for more jobs than modeljobs.
    for (auto &job : rq.Forward) {
        auto coolDeathAnimationJob = std::dynamic_pointer_cast<CoolDeathAnimationJob>(job);
        if (coolDeathAnimationJob) {
            GLuint ShaderHandle = m_CoolDeathAnimProgram->GetHandle(); //---
                                                                       //---
            m_CoolDeathAnimProgram->Bind();
            //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(coolDeathAnimationJob->ModelMatrix));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_Renderer->Camera()->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_Renderer->Camera()->ProjectionMatrix()));
            glUniform4fv(glGetUniformLocation(ShaderHandle, "Color"), 1, glm::value_ptr(coolDeathAnimationJob->Color));
            glUniform3fv(glGetUniformLocation(ShaderHandle, "OriginPos"), 1, glm::value_ptr(Origin));
            glUniform1f(glGetUniformLocation(ShaderHandle, "TimeSinceDeath"), TimeDeath);
            glUniform1f(glGetUniformLocation(ShaderHandle, "EndOfDeath"), EndDeath);

            //TODO: Renderer: bättre textur felhantering samt fler texturer stöd
            if (coolDeathAnimationJob->DiffuseTexture != nullptr) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, coolDeathAnimationJob->DiffuseTexture->m_Texture);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
            }
            glBindVertexArray(coolDeathAnimationJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, coolDeathAnimationJob->Model->ElementBuffer);
            glDrawElementsBaseVertex(GL_TRIANGLES, coolDeathAnimationJob->EndIndex - coolDeathAnimationJob->StartIndex + 1, GL_UNSIGNED_INT, 0, coolDeathAnimationJob->StartIndex);
            continue;
        }
        
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
        if (modelJob) {
            GLuint ShaderHandle = m_BasicForwardProgram->GetHandle(); //---
            GLERROR("1");
            //---
            m_BasicForwardProgram->Bind();
            GLERROR("2.1");
            //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->ModelMatrix));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_Renderer->Camera()->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_Renderer->Camera()->ProjectionMatrix()));
            glUniform4fv(glGetUniformLocation(ShaderHandle, "Color"), 1, glm::value_ptr(modelJob->Color));
            GLERROR("2");
            //TODO: Renderer: bättre textur felhantering samt fler texturer stöd
            if (modelJob->DiffuseTexture != nullptr) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, modelJob->DiffuseTexture->m_Texture);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
            }
            GLERROR("3");
            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElementsBaseVertex(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, 0, modelJob->StartIndex);
            GLERROR("4");
            continue;
        }
    }
    GLERROR("DrawScene Error");
}
