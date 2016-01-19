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
    m_ExplosionEffectProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectProgram");
   /* m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ExplosionEffect.vert.glsl")));
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ExplosionEffect.frag.glsl")));
    m_ExplosionEffectProgram->Compile();
    m_ExplosionEffectProgram->Link();*/
}

void DrawScenePass::Draw(RenderScene& scene)
{

    GLERROR("DrawScenePass::Draw: Pre");

    DrawScenePassState state = DrawScenePassState();
    m_BasicForwardProgram->Bind();

    for (auto &job : scene.ForwardJobs) {
        auto explosionEffectJob = std::dynamic_pointer_cast<ExplosionEffectJob>(job);
        if (explosionEffectJob) {
            GLuint ShaderHandle = m_ExplosionEffectProgram->GetHandle(); //---

            m_ExplosionEffectProgram->Bind();
            //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(explosionEffectJob->Matrix));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
            glUniform4fv(glGetUniformLocation(ShaderHandle, "Color"), 1, glm::value_ptr(explosionEffectJob->Color));
            glUniform3fv(glGetUniformLocation(ShaderHandle, "ExplosionOrigin"), 1, glm::value_ptr(explosionEffectJob->ExplosionOrigin));
            glUniform1f(glGetUniformLocation(ShaderHandle, "TimeSinceDeath"), explosionEffectJob->TimeSinceDeath);
            glUniform1f(glGetUniformLocation(ShaderHandle, "ExplosionDuration"), explosionEffectJob->ExplosionDuration);
            //glUniform1i(glGetUniformLocation(ShaderHandle, "Gravity"), explosionEffectJob->Gravity);
            //glUniform1f(glGetUniformLocation(ShaderHandle, "GravityForce"), explosionEffectJob->GravityForce);
            //glUniform1f(glGetUniformLocation(ShaderHandle, "ObjectRadius"), explosionEffectJob->ObjectRadius);
            glUniform4fv(glGetUniformLocation(ShaderHandle, "EndColor"), 1, glm::value_ptr(explosionEffectJob->EndColor));
            glUniform1i(glGetUniformLocation(ShaderHandle, "Randomness"), explosionEffectJob->Randomness);
            glUniform1fv(glGetUniformLocation(ShaderHandle, "RandomNumbers"), 50, explosionEffectJob->RandomNumbers.data());
            glUniform1f(glGetUniformLocation(ShaderHandle, "RandomnessScalar"), explosionEffectJob->RandomnessScalar);
            glUniform2fv(glGetUniformLocation(ShaderHandle, "Velocity"), 1, glm::value_ptr(explosionEffectJob->Velocity));
            glUniform1i(glGetUniformLocation(ShaderHandle, "ColorByDistance"), explosionEffectJob->ColorByDistance);
            //glUniform1i(glGetUniformLocation(ShaderHandle, "ReverseAnimation"), explosionEffectJob->ReverseAnimation);
            //glUniform1i(glGetUniformLocation(ShaderHandle, "Wireframe"), explosionEffectJob->Wireframe);
            glUniform1i(glGetUniformLocation(ShaderHandle, "ExponentialAccelaration"), explosionEffectJob->ExponentialAccelaration);




            //TODO: Renderer: bättre textur felhantering samt fler texturer stöd
            if (explosionEffectJob->DiffuseTexture != nullptr) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, explosionEffectJob->DiffuseTexture->m_Texture);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
            }
            glBindVertexArray(explosionEffectJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, explosionEffectJob->Model->ElementBuffer);
            glDrawElementsBaseVertex(GL_TRIANGLES, explosionEffectJob->EndIndex - explosionEffectJob->StartIndex + 1, GL_UNSIGNED_INT, 0, explosionEffectJob->StartIndex);
            continue;
        }
        
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
        /*if (modelJob) {
            GLuint ShaderHandle = m_BasicForwardProgram->GetHandle(); //---
            GLERROR("1");
            //---
            GLERROR("2.1");
            //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
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
            //continue;
        }*/
    
    }
    GLERROR("DrawScenePass::Draw: End");
}
