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
    m_WhiteTexture = ResourceManager::Load<Texture>("Textures/Core/White.png");
    m_BlackTexture = ResourceManager::Load<Texture>("Textures/Core/Black.png");
    m_NeutralNormalTexture = ResourceManager::Load<Texture>("Textures/Core/NeutralNormalMap.png");
    m_GreyTexture = ResourceManager::Load<Texture>("Textures/Core/Grey.png");
}

void DrawFinalPass::InitializeFrameBuffers()
{
    glGenRenderbuffers(1, &m_DepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

    GenerateTexture(&m_SceneTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->Resolution().Width, m_Renderer->Resolution().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->Resolution().Width, m_Renderer->Resolution().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->Resolution().Width, m_Renderer->Resolution().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateMipMapTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, glm::vec2(m_Renderer->Resolution().Width, m_Renderer->Resolution().Height), GL_RGB16F, GL_FLOAT, 4);

    m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new RenderBuffer(&m_DepthBuffer, GL_DEPTH_ATTACHMENT)));
    m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_SceneTexture, GL_COLOR_ATTACHMENT0)));
    m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_BloomTexture, GL_COLOR_ATTACHMENT1)));
    m_FinalPassFrameBuffer.Generate();

}

void DrawFinalPass::InitializeShaderPrograms()
{
    m_ForwardPlusProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusProgram");
    m_ForwardPlusProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
    m_ForwardPlusProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlus.frag.glsl")));
    m_ForwardPlusProgram->Compile();
    m_ForwardPlusProgram->BindFragDataLocation(0, "sceneColor");
    m_ForwardPlusProgram->BindFragDataLocation(1, "bloomColor");
    m_ForwardPlusProgram->Link();

    m_ExplosionEffectProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectProgram");
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlus.frag.glsl")));
    m_ExplosionEffectProgram->Compile();
    m_ExplosionEffectProgram->BindFragDataLocation(0, "sceneColor");
    m_ExplosionEffectProgram->BindFragDataLocation(1, "bloomColor");
    m_ExplosionEffectProgram->Link();
}

void DrawFinalPass::Draw(RenderScene& scene)
{
    GLERROR("DrawFinalPass::Draw: Pre");

    DrawFinalPassState* state = new DrawFinalPassState(m_FinalPassFrameBuffer.GetHandle());
    if (scene.ClearDepth) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    DrawModelRenderQueues(scene.OpaqueObjects, scene);
    GLERROR("DrawFinalPass::Draw: OpaqueObjects");
    DrawModelRenderQueues(scene.TransparentObjects, scene);
    GLERROR("DrawFinalPass::Draw: TransparentObjects");

    GLERROR("DrawFinalPass::Draw: END");
    delete state;
}


void DrawFinalPass::ClearBuffer()
{
    m_FinalPassFrameBuffer.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_FinalPassFrameBuffer.Unbind();
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

void DrawFinalPass::DrawModelRenderQueues(std::list<std::shared_ptr<RenderJob>>& job, RenderScene& scene)
{
    GLuint forwardHandle = m_ForwardPlusProgram->GetHandle();
    GLuint explosionHandle = m_ExplosionEffectProgram->GetHandle();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightCullingPass->LightSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightCullingPass->LightGridSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightCullingPass->LightIndexSSBO());

    for(auto &job : job)
    {
        auto explosionEffectJob = std::dynamic_pointer_cast<ExplosionEffectJob>(job);
        if(explosionEffectJob) {
            //Bind program
            m_ExplosionEffectProgram->Bind();

            //Bind uniforms
            BindExplosionUniforms(explosionHandle, explosionEffectJob, scene);

            if (explosionEffectJob->Model->m_RawModel->m_Skeleton != nullptr) {

                if (explosionEffectJob->Animation != nullptr) {
                    std::vector<glm::mat4> frameBones = explosionEffectJob->Skeleton->GetFrameBones(*explosionEffectJob->Animation, explosionEffectJob->AnimationTime);
                    glUniformMatrix4fv(glGetUniformLocation(explosionHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
                }
            }

            //bind textures
            BindExplosionTextures(explosionEffectJob);
            //draw
            glBindVertexArray(explosionEffectJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, explosionEffectJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, explosionEffectJob->EndIndex - explosionEffectJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(explosionEffectJob->StartIndex*sizeof(unsigned int)));

        } else {
            auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
            if (modelJob) {
                //bind forward program
                m_ForwardPlusProgram->Bind();

                //bind uniforms
                BindModelUniforms(forwardHandle, modelJob, scene);

                //bind textures
                BindModelTextures(modelJob);

                if (modelJob->Model->m_RawModel->m_Skeleton != nullptr) {

                    if (modelJob->Animation != nullptr) {
                        std::vector<glm::mat4> frameBones = modelJob->Skeleton->GetFrameBones(*modelJob->Animation, modelJob->AnimationTime);
                        glUniformMatrix4fv(glGetUniformLocation(forwardHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
                    }
                }

                //draw
                glBindVertexArray(modelJob->Model->VAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
                glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex*sizeof(unsigned int)));
                GLERROR("DrawFinalPass::Model: END");
            }
        }
    }

}

void DrawFinalPass::BindExplosionUniforms(GLuint shaderHandle, std::shared_ptr<ExplosionEffectJob>& job, RenderScene& scene)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
    glUniform2f(glGetUniformLocation(shaderHandle, "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(job->Matrix));
    glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(job->Color));
    glUniform3fv(glGetUniformLocation(shaderHandle, "ExplosionOrigin"), 1, glm::value_ptr(job->ExplosionOrigin));
    glUniform1f(glGetUniformLocation(shaderHandle, "TimeSinceDeath"), job->TimeSinceDeath);
    glUniform1f(glGetUniformLocation(shaderHandle, "ExplosionDuration"), job->ExplosionDuration);
    glUniform4fv(glGetUniformLocation(shaderHandle, "EndColor"), 1, glm::value_ptr(job->EndColor));
    glUniform1i(glGetUniformLocation(shaderHandle, "Randomness"), job->Randomness);
    glUniform1f(glGetUniformLocation(shaderHandle, "RandomnessScalar"), job->RandomnessScalar);
    glUniform2fv(glGetUniformLocation(shaderHandle, "Velocity"), 1, glm::value_ptr(job->Velocity));
    glUniform1i(glGetUniformLocation(shaderHandle, "ColorByDistance"), job->ColorByDistance);
    glUniform1i(glGetUniformLocation(shaderHandle, "ExponentialAccelaration"), job->ExponentialAccelaration);
    glUniform4fv(glGetUniformLocation(shaderHandle, "DiffuseColor"), 1, glm::value_ptr(job->DiffuseColor));
    glUniform1fv(glGetUniformLocation(shaderHandle, "RandomNumbers"), 50, job->RandomNumbers.data());
}

void DrawFinalPass::BindModelUniforms(GLuint shaderHandle, std::shared_ptr<ModelJob>& job, RenderScene& scene)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
    glUniform2f(glGetUniformLocation(shaderHandle, "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(job->Matrix));
    glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(job->Color));
    glUniform4fv(glGetUniformLocation(shaderHandle, "DiffuseColor"), 1, glm::value_ptr(job->DiffuseColor));
    glUniform4fv(glGetUniformLocation(shaderHandle, "FillColor"), 1, glm::value_ptr(job->FillColor));
    glUniform1f(glGetUniformLocation(shaderHandle, "FillPercentage"), job->FillPercentage);
}


void DrawFinalPass::BindExplosionTextures(std::shared_ptr<ExplosionEffectJob>& job)
{
    glActiveTexture(GL_TEXTURE0);
    if (job->DiffuseTexture != nullptr) {
        glBindTexture(GL_TEXTURE_2D, job->DiffuseTexture->m_Texture);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
    }
    glActiveTexture(GL_TEXTURE1);
    if (job->IncandescenceTexture != nullptr) {
        glBindTexture(GL_TEXTURE_2D, job->IncandescenceTexture->m_Texture);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_BlackTexture->m_Texture);
    }
}

void DrawFinalPass::BindModelTextures(std::shared_ptr<ModelJob>& job)
{
    glActiveTexture(GL_TEXTURE0);
    if (job->DiffuseTexture != nullptr) {
        glBindTexture(GL_TEXTURE_2D, job->DiffuseTexture->m_Texture);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
    }

    glActiveTexture(GL_TEXTURE1);
    if (job->NormalTexture != nullptr) {
        glBindTexture(GL_TEXTURE_2D, job->NormalTexture->m_Texture); 
    } else {
        glBindTexture(GL_TEXTURE_2D, m_NeutralNormalTexture->m_Texture);
    }

    glActiveTexture(GL_TEXTURE2);
    if (job->SpecularTexture != nullptr) {
        glBindTexture(GL_TEXTURE_2D, job->SpecularTexture->m_Texture);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_GreyTexture->m_Texture);
    }

    glActiveTexture(GL_TEXTURE3);
    if (job->IncandescenceTexture != nullptr) {
        glBindTexture(GL_TEXTURE_2D, job->IncandescenceTexture->m_Texture);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_BlackTexture->m_Texture);
    }
}

