#include "Rendering/DrawFinalPass.h"

DrawFinalPass::DrawFinalPass(IRenderer* renderer, LightCullingPass* lightCullingPass)
{
    //TODO: Make sure that uniforms are not sent into shader if not needed.
    m_Renderer = renderer;
    m_LightCullingPass = lightCullingPass;
    m_ShieldPixelRate = 8;
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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    GLERROR("RenderBuffer generation");

    GenerateTexture(&m_SceneTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewPortSize().Width, m_Renderer->GetViewPortSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateMipMapTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, glm::vec2(m_Renderer->GetViewPortSize().Width, m_Renderer->GetViewPortSize().Height), GL_RGB16F, GL_FLOAT, 4);
    //GenerateTexture(&m_StencilTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_STENCIL, GL_STENCIL_INDEX8, GL_INT);

    m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new RenderBuffer(&m_DepthBuffer, GL_DEPTH_STENCIL_ATTACHMENT)));
    //m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_StencilTexture, GL_STENCIL_ATTACHMENT)));
    m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_SceneTexture, GL_COLOR_ATTACHMENT0)));
    m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_BloomTexture, GL_COLOR_ATTACHMENT1)));
    m_FinalPassFrameBuffer.Generate();
    GLERROR("FBO generation");

    glGenRenderbuffers(1, &m_DepthBufferLowRes);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBufferLowRes);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)(m_Renderer->GetViewportSize().Width/m_ShieldPixelRate), (int)(m_Renderer->GetViewportSize().Height/m_ShieldPixelRate));
    GLERROR("RenderBufferLowRes generation");

    GenerateTexture(&m_SceneTextureLowRes, GL_CLAMP_TO_EDGE, GL_NEAREST, glm::vec2((int)(m_Renderer->GetViewportSize().Width/m_ShieldPixelRate), (int)(m_Renderer->GetViewportSize().Height/m_ShieldPixelRate)), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewPortSize().Width, m_Renderer->GetViewPortSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    GenerateTexture(&m_BloomTextureLowRes, GL_CLAMP_TO_EDGE, GL_NEAREST, glm::vec2((int)(m_Renderer->GetViewportSize().Width/m_ShieldPixelRate), (int)(m_Renderer->GetViewportSize().Height/m_ShieldPixelRate)), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateMipMapTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, glm::vec2(m_Renderer->GetViewPortSize().Width, m_Renderer->GetViewPortSize().Height), GL_RGB16F, GL_FLOAT, 4);
    //GenerateTexture(&m_StencilTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_STENCIL, GL_STENCIL_INDEX8, GL_INT);

    m_FinalPassFrameBufferLowRes.AddResource(std::shared_ptr<BufferResource>(new RenderBuffer(&m_DepthBufferLowRes, GL_DEPTH_STENCIL_ATTACHMENT)));
    //m_FinalPassFrameBufferLowRes.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_StencilTexture, GL_STENCIL_ATTACHMENT)));
    m_FinalPassFrameBufferLowRes.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_SceneTextureLowRes, GL_COLOR_ATTACHMENT0)));
    m_FinalPassFrameBufferLowRes.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_BloomTextureLowRes, GL_COLOR_ATTACHMENT1)));
    m_FinalPassFrameBufferLowRes.Generate();
    GLERROR("FBO2 generation");
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
    GLERROR("Creating forward+ program");

    m_ExplosionEffectProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectProgram");
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
    m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlus.frag.glsl")));
    m_ExplosionEffectProgram->Compile();
    m_ExplosionEffectProgram->BindFragDataLocation(0, "sceneColor");
    m_ExplosionEffectProgram->BindFragDataLocation(1, "bloomColor");
    m_ExplosionEffectProgram->Link();
    GLERROR("Creating explosion program");

    m_ShieldToStencilProgram = ResourceManager::Load<ShaderProgram>("#ShieldToStencilProgram");
    m_ShieldToStencilProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ShieldStencil.vert.glsl")));
    m_ShieldToStencilProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ShieldStencil.frag.glsl")));
    m_ShieldToStencilProgram->Compile();
    m_ShieldToStencilProgram->Link();
    GLERROR("Creating Shield program");

    m_FillDepthBufferProgram = ResourceManager::Load<ShaderProgram>("#FillDepthBufferProgram");
    m_FillDepthBufferProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/FillDepthBuffer.vert.glsl")));
    m_FillDepthBufferProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/FillDepthBuffer.frag.glsl")));
    m_FillDepthBufferProgram->Compile();
    m_FillDepthBufferProgram->Link();
    GLERROR("Creating DepthFill program");
}

void DrawFinalPass::Draw(RenderScene& scene)
{
    GLERROR("Pre");

    DrawFinalPassState* state = new DrawFinalPassState(m_FinalPassFrameBuffer.GetHandle());
    if (scene.ClearDepth) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    //TODO: Do we need check for this or will it be per scene always?
    glClearStencil(0x00);
    glClear(GL_STENCIL_BUFFER_BIT);

    state->StencilMask(0x00);
    DrawModelRenderQueues(scene.Jobs.OpaqueObjects, scene);
    GLERROR("OpaqueObjects");
    DrawModelRenderQueues(scene.Jobs.TransparentObjects, scene);
    GLERROR("TransparentObjects");

    //DrawStencilState* stencilState = new DrawStencilState(m_FinalPassFrameBuffer.GetHandle());
    //Draw shields to stencil pass
    state->StencilFunc(GL_ALWAYS, 1, 0xFF);
    state->StencilMask(0xFF);
    DrawShieldToStencilBuffer(scene.Jobs.ShieldObjects, scene);
    GLERROR("StencilPass");

    //Draw Opaque shielded objects
    state->StencilFunc(GL_NOTEQUAL, 1, 0xFF);
    state->StencilMask(0x00);
    DrawShieldedModelRenderQueue(scene.Jobs.OpaqueShieldedObjects, scene);
    GLERROR("Shielded Opaque object");

    //Draw Transparen Shielded objects
    DrawShieldedModelRenderQueue(scene.Jobs.TransparentShieldedObjects, scene);
    GLERROR("Shielded Transparent objects");

    GLERROR("END");
    delete state;


    DrawFinalPassState* stateLowRes = new DrawFinalPassState(m_FinalPassFrameBufferLowRes.GetHandle());
    //Draw the lowres texture that will be shown behind the shield.
    state->Enable(GL_SCISSOR_TEST);
    state->Enable(GL_DEPTH_TEST);
    //TODO: Viewports and scissor should be in state
    glViewport(0, 0, m_Renderer->GetViewportSize().Width/m_ShieldPixelRate, m_Renderer->GetViewportSize().Height/m_ShieldPixelRate);
    glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);

    glClearStencil(0x00);
    glClear(GL_STENCIL_BUFFER_BIT);

    //TODO: This should not be here...
    state->StencilFunc(GL_ALWAYS, 1, 0xFF);
    state->StencilMask(0x00);
    DrawToDepthBuffer(scene.Jobs.OpaqueObjects, scene);
    DrawToDepthBuffer(scene.Jobs.TransparentObjects, scene);

    //Draw shields to stencil pass
    state->StencilFunc(GL_ALWAYS, 1, 0xFF);
    state->StencilMask(0xFF);
    state->Enable(GL_DEPTH_TEST);
    DrawShieldToStencilBuffer(scene.Jobs.ShieldObjects, scene);
    GLERROR("StencilPass");

    glClear(GL_DEPTH_BUFFER_BIT);

    state->Enable(GL_DEPTH_TEST);
    state->StencilFunc(GL_LEQUAL, 1, 0xFF);
    state->StencilMask(0x00);
    DrawModelRenderQueues(scene.Jobs.OpaqueObjects, scene);
    GLERROR("OpaqueObjects");
    DrawModelRenderQueues(scene.Jobs.TransparentObjects, scene);
    GLERROR("TransparentObjects");
    glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    delete stateLowRes;
}


void DrawFinalPass::ClearBuffer()
{
    m_FinalPassFrameBufferLowRes.Bind();
    glViewport(0, 0, m_Renderer->GetViewportSize().Width/m_ShieldPixelRate, m_Renderer->GetViewportSize().Height/m_ShieldPixelRate);
    glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    m_FinalPassFrameBufferLowRes.Unbind();

    m_FinalPassFrameBuffer.Bind();
    glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
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

void DrawFinalPass::DrawModelRenderQueues(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene)
{
    GLuint forwardHandle = m_ForwardPlusProgram->GetHandle();
    GLERROR("forwardHandle");
    GLuint explosionHandle = m_ExplosionEffectProgram->GetHandle();
    GLERROR("explosionHandle");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightCullingPass->LightSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightCullingPass->LightGridSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightCullingPass->LightIndexSSBO());

    for(auto &job : jobs)
    {
        auto explosionEffectJob = std::dynamic_pointer_cast<ExplosionEffectJob>(job);
        if(explosionEffectJob) {
            //Bind program
            if(GLERROR("Prebind")) {
                continue;
            }
            m_ExplosionEffectProgram->Bind();
            if(GLERROR("BindProgram")) {
                continue;
            }

            glDisable(GL_CULL_FACE);

            //Bind uniforms
            BindExplosionUniforms(explosionHandle, explosionEffectJob, scene);
            if(GLERROR("BindExplosionUniforms")) {
                continue;
            }

            if (explosionEffectJob->Model->m_RawModel->m_Skeleton != nullptr) {

                if (explosionEffectJob->Animation != nullptr) {
                    std::vector<glm::mat4> frameBones = explosionEffectJob->Skeleton->GetFrameBones(*explosionEffectJob->Animation, explosionEffectJob->AnimationTime);
                    glUniformMatrix4fv(glGetUniformLocation(explosionHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
                }
            }
            if(GLERROR("Animation")) {
                continue;
            }

            //bind textures
            BindExplosionTextures(explosionEffectJob);
            if(GLERROR("BindExplosionTextures")) {
                continue;
            }
            //draw
            glBindVertexArray(explosionEffectJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, explosionEffectJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, explosionEffectJob->EndIndex - explosionEffectJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(explosionEffectJob->StartIndex*sizeof(unsigned int)));
            glEnable(GL_CULL_FACE);
            if(GLERROR("explosion effect end")) {
                continue;
            }

        } else {
            auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
            if (modelJob) {
                //bind forward program
                m_ForwardPlusProgram->Bind();
                glUniform2f(glGetUniformLocation(forwardHandle, "ScreenDimensions"), m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);

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
                if(GLERROR("models end")) {
                    continue;
                }
            }
        }
    }

}


void DrawFinalPass::DrawShieldToStencilBuffer(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene)
{
    m_ShieldToStencilProgram->Bind();
    GLuint shaderHandle = m_ShieldToStencilProgram->GetHandle();

    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));

    for (auto &job : jobs) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
        if (modelJob) {
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));


            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex*sizeof(unsigned int)));
            if (GLERROR("models end")) {
                continue;
            }
        }
    }
}

void DrawFinalPass::DrawShieldedModelRenderQueue(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene)
{
    GLuint forwardHandle = m_ForwardPlusProgram->GetHandle();
    GLERROR("forwardHandle");
    GLuint explosionHandle = m_ExplosionEffectProgram->GetHandle();
    GLERROR("explosionHandle");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightCullingPass->LightSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightCullingPass->LightGridSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightCullingPass->LightIndexSSBO());

    for (auto &job : jobs) {
        auto explosionEffectJob = std::dynamic_pointer_cast<ExplosionEffectJob>(job);
        if (explosionEffectJob) {
            //Bind program
            if (GLERROR("Prebind")) {
                continue;
            }
            m_ExplosionEffectProgram->Bind();
            if (GLERROR("BindProgram")) {
                continue;
            }

            glDisable(GL_CULL_FACE);

            //Bind uniforms
            BindExplosionUniforms(explosionHandle, explosionEffectJob, scene);
            if (GLERROR("BindExplosionUniforms")) {
                continue;
            }

            if (explosionEffectJob->Model->m_RawModel->m_Skeleton != nullptr) {

                if (explosionEffectJob->Animation != nullptr) {
                    std::vector<glm::mat4> frameBones = explosionEffectJob->Skeleton->GetFrameBones(*explosionEffectJob->Animation, explosionEffectJob->AnimationTime);
                    glUniformMatrix4fv(glGetUniformLocation(explosionHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
                }
            }
            if (GLERROR("Animation")) {
                continue;
            }

            //bind textures
            BindExplosionTextures(explosionEffectJob);
            if (GLERROR("BindExplosionTextures")) {
                continue;
            }
            //draw
            glBindVertexArray(explosionEffectJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, explosionEffectJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, explosionEffectJob->EndIndex - explosionEffectJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(explosionEffectJob->StartIndex*sizeof(unsigned int)));
            glEnable(GL_CULL_FACE);
            if (GLERROR("explosion effect end")) {
                continue;
            }

        } else {
            auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
            if (modelJob) {
                //bind forward program
                m_ForwardPlusProgram->Bind();
                glUniform2f(glGetUniformLocation(forwardHandle, "ScreenDimensions"), m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);

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
                if (GLERROR("models end")) {
                    continue;
                }
            }
        }
    }
}


void DrawFinalPass::DrawToDepthBuffer(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene)
{
    m_FillDepthBufferProgram->Bind();
    GLuint shaderHandle = m_FillDepthBufferProgram->GetHandle();
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));

    for (auto &job : jobs) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
      
        //bind uniforms
        glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));

        if (modelJob->Model->m_RawModel->m_Skeleton != nullptr) {

            if (modelJob->Animation != nullptr) {
                std::vector<glm::mat4> frameBones = modelJob->Skeleton->GetFrameBones(*modelJob->Animation, modelJob->AnimationTime);
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
            }
        }

        //draw
        glBindVertexArray(modelJob->Model->VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
        glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex*sizeof(unsigned int)));
        if (GLERROR("models end")) {
            continue;
        }
    }

}

void DrawFinalPass::BindExplosionUniforms(GLuint shaderHandle, std::shared_ptr<ExplosionEffectJob>& job, RenderScene& scene)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(job->Matrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));

    glUniform2f(glGetUniformLocation(shaderHandle, "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

    glUniform3fv(glGetUniformLocation(shaderHandle, "ExplosionOrigin"), 1, glm::value_ptr(job->ExplosionOrigin));
    glUniform1f(glGetUniformLocation(shaderHandle, "TimeSinceDeath"), job->TimeSinceDeath);
    glUniform1f(glGetUniformLocation(shaderHandle, "ExplosionDuration"), job->ExplosionDuration);
    glUniform4fv(glGetUniformLocation(shaderHandle, "EndColor"), 1, glm::value_ptr(job->EndColor));
    glUniform1i(glGetUniformLocation(shaderHandle, "Randomness"), job->Randomness);
    glUniform1fv(glGetUniformLocation(shaderHandle, "RandomNumbers"), 50, job->RandomNumbers.data());
    glUniform1f(glGetUniformLocation(shaderHandle, "RandomnessScalar"), job->RandomnessScalar);
    glUniform2fv(glGetUniformLocation(shaderHandle, "Velocity"), 1, glm::value_ptr(job->Velocity));
    glUniform1i(glGetUniformLocation(shaderHandle, "ColorByDistance"), job->ColorByDistance);
    glUniform1i(glGetUniformLocation(shaderHandle, "ExponentialAccelaration"), job->ExponentialAccelaration);

    glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(job->Color));
    glUniform4fv(glGetUniformLocation(shaderHandle, "DiffuseColor"), 1, glm::value_ptr(job->DiffuseColor));
    glUniform4fv(glGetUniformLocation(shaderHandle, "FillColor"), 1, glm::value_ptr(job->FillColor));
    glUniform1f(glGetUniformLocation(shaderHandle, "FillPercentage"), job->FillPercentage);
    glUniform4fv(glGetUniformLocation(shaderHandle, "AmbientColor"), 1, glm::value_ptr(scene.AmbientColor));
    GLERROR("END");
}

void DrawFinalPass::BindModelUniforms(GLuint shaderHandle, std::shared_ptr<ModelJob>& job, RenderScene& scene)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(job->Matrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));

    glUniform2f(glGetUniformLocation(shaderHandle, "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

    glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(job->Color));
    glUniform4fv(glGetUniformLocation(shaderHandle, "DiffuseColor"), 1, glm::value_ptr(job->DiffuseColor));
    glUniform4fv(glGetUniformLocation(shaderHandle, "FillColor"), 1, glm::value_ptr(job->FillColor));
    glUniform1f(glGetUniformLocation(shaderHandle, "FillPercentage"), job->FillPercentage);
    glUniform4fv(glGetUniformLocation(shaderHandle, "AmbientColor"), 1, glm::value_ptr(scene.AmbientColor));

    GLERROR("END");
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

