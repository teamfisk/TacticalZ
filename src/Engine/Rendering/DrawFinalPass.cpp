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
}

void DrawFinalPass::InitializeShaderPrograms()
{
    m_ForwardPlusProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusProgram");
    m_ForwardPlusProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
    m_ForwardPlusProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlus.frag.glsl")));
    m_ForwardPlusProgram->Compile();
    m_ForwardPlusProgram->Link();

    m_ExplosionEffectProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectProgram");
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlus.frag.glsl")));
    m_ExplosionEffectProgram->Compile();
    m_ExplosionEffectProgram->Link();
}

void DrawFinalPass::Draw(RenderScene& scene)
{
    GLERROR("DrawFinalPass::Draw: Pre");

    DrawFinalPassState state;
    GLuint shaderHandle;
    

    //TODO: Render: Add code for more jobs than modeljobs.
    for (auto &job : scene.ForwardJobs) {

        auto explosionEffectJob = std::dynamic_pointer_cast<ExplosionEffectJob>(job);
        if (explosionEffectJob) {
            m_ExplosionEffectProgram->Bind();
            shaderHandle = m_ExplosionEffectProgram->GetHandle(); //---

            GLERROR("DrawFinalPass::ExplosionEffect: ENDsdufhsdilfuh");

            //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(explosionEffectJob->Matrix));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
            glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(explosionEffectJob->Color));
            glUniform3fv(glGetUniformLocation(shaderHandle, "ExplosionOrigin"), 1, glm::value_ptr(explosionEffectJob->ExplosionOrigin));
            glUniform1f(glGetUniformLocation(shaderHandle, "TimeSinceDeath"), explosionEffectJob->TimeSinceDeath);
            glUniform1f(glGetUniformLocation(shaderHandle, "ExplosionDuration"), explosionEffectJob->ExplosionDuration);
            glUniform4fv(glGetUniformLocation(shaderHandle, "EndColor"), 1, glm::value_ptr(explosionEffectJob->EndColor));
            glUniform1i(glGetUniformLocation(shaderHandle, "Randomness"), explosionEffectJob->Randomness);
            glUniform1fv(glGetUniformLocation(shaderHandle, "RandomNumbers"), 50, explosionEffectJob->RandomNumbers.data());
            glUniform1f(glGetUniformLocation(shaderHandle, "RandomnessScalar"), explosionEffectJob->RandomnessScalar);
            glUniform2fv(glGetUniformLocation(shaderHandle, "Velocity"), 1, glm::value_ptr(explosionEffectJob->Velocity));
            glUniform1i(glGetUniformLocation(shaderHandle, "ColorByDistance"), explosionEffectJob->ColorByDistance);
            glUniform1i(glGetUniformLocation(shaderHandle, "ExponentialAccelaration"), explosionEffectJob->ExponentialAccelaration);
            GLERROR("DrawFinalPass::ExplosionEffect: asdasdasdasdasdasd");

            //TODO: Renderer: bättre textur felhantering samt fler texturer stöd
            if (explosionEffectJob->DiffuseTexture != nullptr) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, explosionEffectJob->DiffuseTexture->m_Texture);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
            }
            GLERROR("DrawFinalPass::ExplosionEffect: END1");
            glBindVertexArray(explosionEffectJob->Model->VAO);
            GLERROR("DrawFinalPass::ExplosionEffect: END2");
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, explosionEffectJob->Model->ElementBuffer);
            GLERROR("DrawFinalPass::ExplosionEffect: END3");
            glDrawElementsBaseVertex(GL_TRIANGLES, explosionEffectJob->EndIndex - explosionEffectJob->StartIndex + 1, GL_UNSIGNED_INT, 0, explosionEffectJob->StartIndex);
            GLERROR("DrawFinalPass::ExplosionEffect: END");
            m_ExplosionEffectProgram->Unbind();
            continue;
        } else {
             auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
             if (modelJob) {
                 m_ForwardPlusProgram->Bind();
                 shaderHandle = m_ForwardPlusProgram->GetHandle();

                 glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightCullingPass->LightSSBO());
                 glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightCullingPass->LightGridSSBO());
                 glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightCullingPass->LightIndexSSBO());

                 glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_Renderer->Camera()->ViewMatrix()));
                 glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_Renderer->Camera()->ProjectionMatrix()));
                 glUniform2f(glGetUniformLocation(shaderHandle, "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

                 //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
                 glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                 glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(modelJob->Color));

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
                 GLERROR("DrawFinalPass::Model: END");
                 m_ForwardPlusProgram->Unbind();
                 continue;
             }
        }
        
    }
    GLERROR("DrawFinalPass::Draw: END");

}
