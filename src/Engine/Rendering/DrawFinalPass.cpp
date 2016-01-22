#include "Rendering/DrawFinalPass.h"

DrawFinalPass::DrawFinalPass(IRenderer* renderer, LightCullingPass* lightCullingPass)
{
    m_Renderer = renderer;
    m_LightCullingPass = lightCullingPass;
    InitializeTextures();
    InitializeShaderPrograms();
}

void DrawFinalPass::InitializeTextures()
{
    m_WhiteTexture = ResourceManager::Load<Texture>("Textures/Core/Blank.png");
	m_NeutralNormalMap = ResourceManager::Load<Texture>("Textures/Core/NeutralNormalMap.png");
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

    DrawFinalPassState state;

    m_ForwardPlusProgram->Bind();
    GLuint shaderHandle = m_ForwardPlusProgram->GetHandle();

    if (scene.ClearDepth) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightCullingPass->LightSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightCullingPass->LightGridSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightCullingPass->LightIndexSSBO());

    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
    glUniform2f(glGetUniformLocation(shaderHandle, "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

    //TODO: Render: Add code for more jobs than modeljobs.
    for (auto &job : scene.ForwardJobs) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
        if(modelJob) {
            //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
            glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(modelJob->Color));

            if(modelJob->DiffuseTexture != nullptr) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, modelJob->DiffuseTexture->m_Texture);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
            }

			if (modelJob->NormalTexture != nullptr) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, modelJob->NormalTexture->m_Texture);
			}
			else {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_NeutralNormalMap->m_Texture);
			}

            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElementsBaseVertex(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, 0, modelJob->StartIndex);

            continue;
        }
    }
    GLERROR("DrawFinalPass::Draw: END");

}
