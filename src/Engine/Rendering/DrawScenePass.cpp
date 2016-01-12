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
}

void DrawScenePass::Draw(RenderFrame& rf)
{
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLERROR("Renderer::Draw PickingPass");

    DrawScenePassState state;
    for (auto scene : rf.RenderScenes) {

        for (auto &job : scene->ForwardJobs) {
            auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
            if (modelJob) {
                GLuint ShaderHandle = m_BasicForwardProgram->GetHandle();

                m_BasicForwardProgram->Bind();
                //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
                glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "Matrix"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniform4fv(glGetUniformLocation(ShaderHandle, "Color"), 1, glm::value_ptr(modelJob->Color));

                //TODO: Renderer: bättre textur felhantering samt fler texturer stöd
                if (modelJob->DiffuseTexture != nullptr) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, modelJob->DiffuseTexture->m_Texture);
                } else {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
                }

                glBindVertexArray(modelJob->Model->VAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
                glDrawElementsBaseVertex(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, 0, modelJob->StartIndex);

                continue;
            }
        }
    }
    GLERROR("DrawScene Error");
}
