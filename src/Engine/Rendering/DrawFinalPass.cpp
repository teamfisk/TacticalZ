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
    m_WhiteTexture = CommonFunctions::LoadTexture("Textures/Core/White.png", false);
    m_BlackTexture = CommonFunctions::LoadTexture("Textures/Core/Black.png", false);
    m_NeutralNormalTexture = CommonFunctions::LoadTexture("Textures/Core/NeutralNormalMap.png", false);
    m_GreyTexture = CommonFunctions::LoadTexture("Textures/Core/Grey.png", false);
    m_ErrorTexture = CommonFunctions::LoadTexture("Textures/Core/ErrorTexture.png", false);
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

    m_SpriteProgram = ResourceManager::Load<ShaderProgram>("#SpriteProgram");
    m_SpriteProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Sprite.vert.glsl")));
    m_SpriteProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Sprite.frag.glsl")));
    m_SpriteProgram->Compile();
    m_SpriteProgram->BindFragDataLocation(0, "sceneColor");
    m_SpriteProgram->BindFragDataLocation(1, "bloomColor");
    m_SpriteProgram->Link();
    GLERROR("Creating sprite program");
	m_ForwardPlusSplatMapProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusSplatMapProgram");
	m_ForwardPlusSplatMapProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
	m_ForwardPlusSplatMapProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusSplatMapRGB.frag.glsl")));
	m_ForwardPlusSplatMapProgram->Compile();
	m_ForwardPlusSplatMapProgram->BindFragDataLocation(0, "sceneColor");
	m_ForwardPlusSplatMapProgram->BindFragDataLocation(1, "bloomColor");
	m_ForwardPlusSplatMapProgram->Link();
	GLERROR("Creating Forward SplatMap program");

	m_ExplosionEffectSplatMapProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectSplatMapProgram");
	m_ExplosionEffectSplatMapProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
	m_ExplosionEffectSplatMapProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
	m_ExplosionEffectSplatMapProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusSplatMapRGB.frag.glsl")));
	m_ExplosionEffectSplatMapProgram->Compile();
	m_ExplosionEffectSplatMapProgram->BindFragDataLocation(0, "sceneColor");
	m_ExplosionEffectSplatMapProgram->BindFragDataLocation(1, "bloomColor");
	m_ExplosionEffectSplatMapProgram->Link();
	GLERROR("Creating explosion SplatMap program");

	m_ForwardPlusSkinnedProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusSkinnedProgram");
	m_ForwardPlusSkinnedProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlusSkinned.vert.glsl")));
	m_ForwardPlusSkinnedProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlus.frag.glsl")));
	m_ForwardPlusSkinnedProgram->Compile();
	m_ForwardPlusSkinnedProgram->BindFragDataLocation(0, "sceneColor");
	m_ForwardPlusSkinnedProgram->BindFragDataLocation(1, "bloomColor");
	m_ForwardPlusSkinnedProgram->Link();
	GLERROR("Creating forward+ Skinned program");

	m_ExplosionEffectSkinnedProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectSkinnedProgram");
	m_ExplosionEffectSkinnedProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlusSkinned.vert.glsl")));
	m_ExplosionEffectSkinnedProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
	m_ExplosionEffectSkinnedProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlus.frag.glsl")));
	m_ExplosionEffectSkinnedProgram->Compile();
	m_ExplosionEffectSkinnedProgram->BindFragDataLocation(0, "sceneColor");
	m_ExplosionEffectSkinnedProgram->BindFragDataLocation(1, "bloomColor");
	m_ExplosionEffectSkinnedProgram->Link();
	GLERROR("Creating explosion Skinned program");

	m_ExplosionEffectSplatMapSkinnedProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectSplatMapSkinnedProgram");
	m_ExplosionEffectSplatMapSkinnedProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlusSkinned.vert.glsl")));
	m_ExplosionEffectSplatMapSkinnedProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusSplatMapRGB.frag.glsl")));
	m_ExplosionEffectSplatMapSkinnedProgram->Compile();
	m_ExplosionEffectSplatMapSkinnedProgram->BindFragDataLocation(0, "sceneColor");
	m_ExplosionEffectSplatMapSkinnedProgram->BindFragDataLocation(1, "bloomColor");
	m_ExplosionEffectSplatMapSkinnedProgram->Link();
	GLERROR("Creating Forward SplatMap Skinned program");

	m_ForwardPlusSplatMapSkinnedProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusSplatMapSkinnedProgram");
	m_ForwardPlusSplatMapSkinnedProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlusSkinned.vert.glsl")));
	m_ForwardPlusSplatMapSkinnedProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusSplatMapRGB.frag.glsl")));
	m_ForwardPlusSplatMapSkinnedProgram->Compile();
	m_ForwardPlusSplatMapSkinnedProgram->BindFragDataLocation(0, "sceneColor");
	m_ForwardPlusSplatMapSkinnedProgram->BindFragDataLocation(1, "bloomColor");
	m_ForwardPlusSplatMapSkinnedProgram->Link();
	GLERROR("Creating Forward SplatMap Skinned program");
	
    m_ShieldToStencilProgram = ResourceManager::Load<ShaderProgram>("#ShieldToStencilProgram");
    m_ShieldToStencilProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ShieldStencil.vert.glsl")));
    m_ShieldToStencilProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ShieldStencil.frag.glsl")));
    m_ShieldToStencilProgram->Compile();
    m_ShieldToStencilProgram->Link();
    GLERROR("Creating Shield program");

    m_ShieldToStencilSkinnedProgram = ResourceManager::Load<ShaderProgram>("#ShieldToStencilProgramSkinned");
    m_ShieldToStencilSkinnedProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ShieldStencilSkinned.vert.glsl")));
    m_ShieldToStencilSkinnedProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ShieldStencil.frag.glsl")));
    m_ShieldToStencilSkinnedProgram->Compile();
    m_ShieldToStencilSkinnedProgram->Link();
    GLERROR("Creating Shield Skinned program");

    m_FillDepthBufferProgram = ResourceManager::Load<ShaderProgram>("#FillDepthBufferProgram");
    m_FillDepthBufferProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/FillDepthBuffer.vert.glsl")));
    m_FillDepthBufferProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/FillDepthBuffer.frag.glsl")));
    m_FillDepthBufferProgram->Compile();
    m_FillDepthBufferProgram->Link();
    GLERROR("Creating DepthFill program");

    m_FillDepthBufferSkinnedProgram = ResourceManager::Load<ShaderProgram>("#FillDepthBufferProgramSkinned");
    m_FillDepthBufferSkinnedProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/FillDepthBufferSkinned.vert.glsl")));
    m_FillDepthBufferSkinnedProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/FillDepthBuffer.frag.glsl")));
    m_FillDepthBufferSkinnedProgram->Compile();
    m_FillDepthBufferSkinnedProgram->Link();
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

    //Fill depth buffer


    state->StencilMask(0x00);
    DrawModelRenderQueues(scene.Jobs.OpaqueObjects, scene);
    GLERROR("OpaqueObjects");
    DrawModelRenderQueues(scene.Jobs.TransparentObjects, scene);
    GLERROR("TransparentObjects");
    DrawSprites(scene.Jobs.SpriteJob, scene);
    GLERROR("SpriteJobs");

    //DrawStencilState* stencilState = new DrawStencilState(m_FinalPassFrameBuffer.GetHandle());
    //Draw shields to stencil pass
    state->StencilFunc(GL_ALWAYS, 1, 0xFF);
    state->StencilMask(0xFF);
    DrawShieldToStencilBuffer(scene.Jobs.ShieldObjects, scene);
    GLERROR("StencilPass");

    //Draw Opaque shielded objects
    state->StencilFunc(GL_NOTEQUAL, 1, 0xFF);
    state->StencilMask(0x00);
    DrawModelRenderQueues(scene.Jobs.OpaqueShieldedObjects, scene); //might need changing
    GLERROR("Shielded Opaque object");

    //Draw Transparen Shielded objects
    DrawModelRenderQueues(scene.Jobs.TransparentShieldedObjects, scene); //might need changing
    GLERROR("Shielded Transparent objects");

    GLERROR("END");
    delete state;


    DrawFinalPassState* stateLowRes = new DrawFinalPassState(m_FinalPassFrameBufferLowRes.GetHandle());
    //Draw the lowres texture that will be shown behind the shield.
    stateLowRes->Enable(GL_SCISSOR_TEST);
    stateLowRes->Enable(GL_DEPTH_TEST);
    //TODO: Viewports and scissor should be in state
    glViewport(0, 0, m_Renderer->GetViewportSize().Width/m_ShieldPixelRate, m_Renderer->GetViewportSize().Height/m_ShieldPixelRate);
    glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);

    glClearStencil(0x00);
    glClear(GL_STENCIL_BUFFER_BIT);

    //TODO: This should not be here...
    stateLowRes->StencilFunc(GL_ALWAYS, 1, 0xFF);
    stateLowRes->StencilMask(0x00);
    DrawToDepthBuffer(scene.Jobs.OpaqueObjects, scene);
    DrawToDepthBuffer(scene.Jobs.TransparentObjects, scene);

    //Draw shields to stencil pass
    stateLowRes->StencilFunc(GL_ALWAYS, 1, 0xFF);
    stateLowRes->StencilMask(0xFF);
    stateLowRes->Enable(GL_DEPTH_TEST);
    DrawShieldToStencilBuffer(scene.Jobs.ShieldObjects, scene);
    GLERROR("StencilPass");

    glClear(GL_DEPTH_BUFFER_BIT);

    stateLowRes->Enable(GL_DEPTH_TEST);
    stateLowRes->StencilFunc(GL_LEQUAL, 1, 0xFF);
    stateLowRes->StencilMask(0x00);
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
	GLuint explosionSplatMapHandle = m_ExplosionEffectSplatMapProgram->GetHandle();
	GLERROR("explosionSplatMapHandle");
	GLuint forwardSplatHandle = m_ForwardPlusSplatMapProgram->GetHandle();
	GLERROR("forwardSplatHandle");
	GLuint forwardSkinnedHandle = m_ForwardPlusSkinnedProgram->GetHandle();
	GLERROR("forwardSkinnedHandle");
	GLuint explosionSkinnedHandle = m_ExplosionEffectSkinnedProgram->GetHandle();
	GLERROR("explosionSkinnedHandle");
	GLuint explosionSplatMapSkinnedHandle = m_ExplosionEffectSplatMapSkinnedProgram->GetHandle();
	GLERROR("explosionSplatMapSkinnedHandle");
	GLuint forwardSplatMapSkinnedHandle = m_ForwardPlusSplatMapSkinnedProgram->GetHandle();
	GLERROR("forwardSplatSkinnedHandle");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightCullingPass->LightSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightCullingPass->LightGridSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightCullingPass->LightIndexSSBO());

    for (auto &job : jobs) {
        auto explosionEffectJob = std::dynamic_pointer_cast<ExplosionEffectJob>(job);
        if (explosionEffectJob) {
            switch (explosionEffectJob->Type) {
            case RawModel::MaterialType::Basic:
            case RawModel::MaterialType::SingleTextures:
            {
                if (explosionEffectJob->Model->IsSkinned()) {
                    m_ExplosionEffectSkinnedProgram->Bind();
                    GLERROR("Bind ExplosionEffectSkinned program");
                    //bind uniforms
                    BindExplosionUniforms(explosionSkinnedHandle, explosionEffectJob, scene);
                    //bind textures
                    BindExplosionTextures(explosionSkinnedHandle, explosionEffectJob);
                    std::vector<glm::mat4> frameBones;
                    frameBones = explosionEffectJob->BlendTree->GetFinalPose();
                    glUniformMatrix4fv(glGetUniformLocation(explosionSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
                } else {
                    m_ExplosionEffectProgram->Bind();
                    GLERROR("Bind ExplosionEffect program");
                    //bind uniforms
                    BindExplosionUniforms(explosionHandle, explosionEffectJob, scene);
                    //bind textures
                    BindExplosionTextures(explosionHandle, explosionEffectJob);
                }
                break;
            }
            case RawModel::MaterialType::SplatMapping:
            {
                if (explosionEffectJob->Model->IsSkinned()) {
                    m_ExplosionEffectSplatMapSkinnedProgram->Bind();
                    GLERROR("Bind ExplosionEffectSplatMapSkinned program");
                    //bind uniforms
                    BindExplosionUniforms(explosionSplatMapSkinnedHandle, explosionEffectJob, scene);
                    //bind textures
                    BindExplosionTextures(explosionSplatMapSkinnedHandle, explosionEffectJob);
                    GLERROR("asdasd");
                    std::vector<glm::mat4> frameBones;
                    frameBones = explosionEffectJob->BlendTree->GetFinalPose();
                    glUniformMatrix4fv(glGetUniformLocation(explosionSplatMapSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

                } else {
                    m_ExplosionEffectSplatMapProgram->Bind();
                    GLERROR("Bind ExplosionEffectSplatMap program");
                    //bind uniforms
                    //bind uniforms
                    BindExplosionUniforms(explosionSplatMapHandle, explosionEffectJob, scene);
                    //bind textures
                    BindExplosionTextures(explosionSplatMapHandle, explosionEffectJob);
                    GLERROR("asdasd");
                }
                break;
            }
            }
            glDisable(GL_CULL_FACE);

            //draw
            glBindVertexArray(explosionEffectJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, explosionEffectJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, explosionEffectJob->EndIndex - explosionEffectJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(explosionEffectJob->StartIndex*sizeof(unsigned int)));
            glEnable(GL_CULL_FACE);
            GLERROR("explosion effect end");
            } else {
                auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
                if (modelJob) {
                    //bind forward program
                    //TODO: JOHAN/TOBIAS: Bind shader based on ModelJob->ShaderID;
                    switch (modelJob->Type) {
                    case RawModel::MaterialType::Basic:
                    case RawModel::MaterialType::SingleTextures:
                    {
                        if (modelJob->Model->IsSkinned()) {
                            m_ForwardPlusSkinnedProgram->Bind();
                            GLERROR("Bind ForwardPlusSkinnedProgram");
                            //bind uniforms
                            BindModelUniforms(forwardSkinnedHandle, modelJob, scene);
                            //bind textures
                            BindModelTextures(forwardSkinnedHandle, modelJob);
                            std::vector<glm::mat4> frameBones;
                            frameBones = modelJob->BlendTree->GetFinalPose();
                            glUniformMatrix4fv(glGetUniformLocation(forwardSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

                        } else {
                            m_ForwardPlusProgram->Bind();
                            GLERROR("Bind ForwardPlusProgram");
                            //bind uniforms
                            BindModelUniforms(forwardHandle, modelJob, scene);
                            //bind textures
                            BindModelTextures(forwardHandle, modelJob);
                        }
                        break;
                    }
                    case RawModel::MaterialType::SplatMapping:
                    {
                        if (modelJob->Model->IsSkinned()) {
                            m_ForwardPlusSplatMapSkinnedProgram->Bind();
                            GLERROR("Bind SplatMap program");
                            //bind uniforms
                            BindModelUniforms(forwardSplatMapSkinnedHandle, modelJob, scene);
                            //bind textures
                            BindModelTextures(forwardSplatMapSkinnedHandle, modelJob);
                            GLERROR("asdasd");
                            std::vector<glm::mat4> frameBones;
                            frameBones = modelJob->BlendTree->GetFinalPose();
                            glUniformMatrix4fv(glGetUniformLocation(forwardSplatMapSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

                        } else {
                            m_ForwardPlusSplatMapProgram->Bind();
                            GLERROR("Bind SplatMap program");
                            //bind uniforms
                            BindModelUniforms(forwardSplatHandle, modelJob, scene);
                            //bind textures
                            BindModelTextures(forwardSplatHandle, modelJob);
                            GLERROR("asdasd");
                        }
                        break;
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


void DrawFinalPass::DrawShieldToStencilBuffer(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene)
{


    for (auto &job : jobs) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
        if (modelJob) {

            if(modelJob->Model->IsSkinned()) {
                m_ShieldToStencilSkinnedProgram->Bind();
                GLuint shaderHandle = m_ShieldToStencilSkinnedProgram->GetHandle();

                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
                std::vector<glm::mat4> frameBones;
                frameBones = modelJob->BlendTree->GetFinalPose();
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
            } else {
                m_ShieldToStencilProgram->Bind();
                GLuint shaderHandle = m_ShieldToStencilProgram->GetHandle();

                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));

            }

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

            std::vector<glm::mat4> frameBones;
            frameBones = explosionEffectJob->BlendTree->GetFinalPose();
            glUniformMatrix4fv(glGetUniformLocation(explosionHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

            if (GLERROR("Animation")) {
                continue;
            }

            //bind textures
            BindExplosionTextures(explosionHandle, explosionEffectJob);
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
                BindModelTextures(forwardHandle ,modelJob);

                std::vector<glm::mat4> frameBones;
                frameBones = modelJob->BlendTree->GetFinalPose();
                glUniformMatrix4fv(glGetUniformLocation(forwardHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));


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
    
   
    for (auto &job : jobs) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
      
        if(modelJob->Model->IsSkinned()) {
            m_FillDepthBufferSkinnedProgram->Bind();
            GLuint shaderHandle = m_FillDepthBufferSkinnedProgram->GetHandle();
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));

            std::vector<glm::mat4> frameBones;
            frameBones = modelJob->BlendTree->GetFinalPose();
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

        } else {
            m_FillDepthBufferProgram->Bind();
            GLuint shaderHandle = m_FillDepthBufferProgram->GetHandle();
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
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


void DrawFinalPass::DrawSprites(std::list<std::shared_ptr<RenderJob>>&jobs, RenderScene& scene)
{
    m_SpriteProgram->Bind();

    GLuint shaderHandle = m_SpriteProgram->GetHandle();

    for(auto& job : jobs) {
        auto spriteJob = std::dynamic_pointer_cast<SpriteJob>(job);
        RenderState jobState;

        if (spriteJob) {
            if(spriteJob->Depth == 0) {
                jobState.Disable(GL_DEPTH_TEST);
            }
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(spriteJob->Matrix));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
            glUniform3fv(glGetUniformLocation(shaderHandle, "CameraPos"), 1, glm::value_ptr(scene.Camera->Position()));
            glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(spriteJob->Color));
            glUniform4fv(glGetUniformLocation(shaderHandle, "FillColor"), 1, glm::value_ptr(spriteJob->FillColor));
            glUniform1f(glGetUniformLocation(shaderHandle, "FillPercentage"), spriteJob->FillPercentage);

            glActiveTexture(GL_TEXTURE0);
            if (spriteJob->DiffuseTexture != nullptr) {
                glBindTexture(GL_TEXTURE_2D, spriteJob->DiffuseTexture->m_Texture);
            } else {
                glBindTexture(GL_TEXTURE_2D, m_ErrorTexture->m_Texture);
            }

            glActiveTexture(GL_TEXTURE1);
            if (spriteJob->IncandescenceTexture != nullptr) {
                glBindTexture(GL_TEXTURE_2D, spriteJob->IncandescenceTexture->m_Texture);
            } else {
                glBindTexture(GL_TEXTURE_2D, m_BlackTexture->m_Texture);
            }


            glBindVertexArray(spriteJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, spriteJob->EndIndex - spriteJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(spriteJob->StartIndex*sizeof(unsigned int)));
        }
    }

    

   // m_SpriteProgram->Unbind();
}

void DrawFinalPass::BindExplosionUniforms(GLuint shaderHandle, std::shared_ptr<ExplosionEffectJob>& job, RenderScene& scene)
{
	GLERROR("Bind 1 uniform");
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(job->Matrix));
	GLERROR("Bind 2 uniform");
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
	GLERROR("Bind 3 uniform");
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
	GLERROR("Bind 4 uniform");

    glUniform2f(glGetUniformLocation(shaderHandle, "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);
	GLERROR("Bind 5 uniform");

    glUniform3fv(glGetUniformLocation(shaderHandle, "ExplosionOrigin"), 1, glm::value_ptr(job->ExplosionOrigin));
	GLERROR("Bind 6 uniform");
    glUniform1f(glGetUniformLocation(shaderHandle, "TimeSinceDeath"), job->TimeSinceDeath);
	GLERROR("Bind 7 uniform");
    glUniform1f(glGetUniformLocation(shaderHandle, "ExplosionDuration"), job->ExplosionDuration);
	GLERROR("Bind 8 uniform");
    glUniform4fv(glGetUniformLocation(shaderHandle, "EndColor"), 1, glm::value_ptr(job->EndColor));
	GLERROR("Bind 9 uniform");
    glUniform1i(glGetUniformLocation(shaderHandle, "Randomness"), job->Randomness);
	GLERROR("Bind 10 uniform");
    glUniform1fv(glGetUniformLocation(shaderHandle, "RandomNumbers"), 50, job->RandomNumbers.data());
	GLERROR("Bind 11 uniform");
    glUniform1f(glGetUniformLocation(shaderHandle, "RandomnessScalar"), job->RandomnessScalar);
	GLERROR("Bind 12 uniform");
    glUniform2fv(glGetUniformLocation(shaderHandle, "Velocity"), 1, glm::value_ptr(job->Velocity));
	GLERROR("Bind 13 uniform");
    glUniform1i(glGetUniformLocation(shaderHandle, "ColorByDistance"), job->ColorByDistance);
	GLERROR("Bind 14 uniform");
    glUniform1i(glGetUniformLocation(shaderHandle, "ExponentialAccelaration"), job->ExponentialAccelaration);
	GLERROR("Bind 15 uniform");

    glUniform4fv(glGetUniformLocation(shaderHandle, "Color"), 1, glm::value_ptr(job->Color));
	GLERROR("Bind 16 uniform");
    glUniform4fv(glGetUniformLocation(shaderHandle, "DiffuseColor"), 1, glm::value_ptr(job->DiffuseColor));
	GLERROR("Bind 17 uniform");
    glUniform4fv(glGetUniformLocation(shaderHandle, "FillColor"), 1, glm::value_ptr(job->FillColor));
	GLERROR("Bind 18 uniform");
    glUniform1f(glGetUniformLocation(shaderHandle, "FillPercentage"), job->FillPercentage);
	GLERROR("Bind 19 uniform");
    glUniform4fv(glGetUniformLocation(shaderHandle, "AmbientColor"), 1, glm::value_ptr(scene.AmbientColor));
    GLERROR("END");
}

void DrawFinalPass::BindModelUniforms(GLuint shaderHandle, std::shared_ptr<ModelJob>& job, RenderScene& scene)
{
	GLERROR("Bind 1 uniform");
	GLint Location_M = glGetUniformLocation(shaderHandle, "M");
	glUniformMatrix4fv(Location_M, 1, GL_FALSE, glm::value_ptr(job->Matrix));
	GLERROR("Bind 2 uniform");
	GLint Location_V = glGetUniformLocation(shaderHandle, "V");
	glUniformMatrix4fv(Location_V, 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
	GLERROR("Bind 3 uniform");
	GLint Location_P = glGetUniformLocation(shaderHandle, "P");
	glUniformMatrix4fv(Location_P, 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
	GLERROR("Bind 4 uniform");

	GLint Location_ScreenDimensions = glGetUniformLocation(shaderHandle, "ScreenDimensions");
	glUniform2f(Location_ScreenDimensions, m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);
	GLERROR("Bind 5 uniform");

	GLint Location_FillPercentage = glGetUniformLocation(shaderHandle, "FillPercentage");
	glUniform1f(Location_FillPercentage, job->FillPercentage);
	GLERROR("Bind 6 uniform");
	GLint Location_DiffuseColor = glGetUniformLocation(shaderHandle, "DiffuseColor");
	glUniform4fv(Location_DiffuseColor, 1, glm::value_ptr(job->DiffuseColor));
	GLERROR("Bind 7 uniform");
	GLint Location_FillColor = glGetUniformLocation(shaderHandle, "FillColor");
	glUniform4fv(Location_FillColor, 1, glm::value_ptr(job->FillColor));
	GLERROR("Bind 8 uniform");
	GLint Location_Color = glGetUniformLocation(shaderHandle, "Color");
	glUniform4fv(Location_Color, 1, glm::value_ptr(job->Color));
	GLERROR("Bind 9 uniform");
	GLint Location_AmbientColor = glGetUniformLocation(shaderHandle, "AmbientColor");
	glUniform4fv(Location_AmbientColor, 1, glm::value_ptr(scene.AmbientColor));

	GLERROR("END");
}

void DrawFinalPass::BindExplosionTextures(GLuint shaderHandle, std::shared_ptr<ExplosionEffectJob>& job)
{
	switch (job->Type) {
	case RawModel::MaterialType::SingleTextures:
	case RawModel::MaterialType::Basic:
	{
		glActiveTexture(GL_TEXTURE0);
		if (job->DiffuseTexture.size() > 0 && job->DiffuseTexture[0]->Texture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, job->DiffuseTexture[0]->Texture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(job->DiffuseTexture[0]->UVRepeat));
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
		}

		glActiveTexture(GL_TEXTURE1);
		if (job->NormalTexture.size() > 0 && job->NormalTexture[0]->Texture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, job->NormalTexture[0]->Texture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "NormalUVRepeat"), 1, glm::value_ptr(job->NormalTexture[0]->UVRepeat));
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_NeutralNormalTexture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "NormalUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
		}

		glActiveTexture(GL_TEXTURE2);
		if (job->SpecularTexture.size() > 0 && job->SpecularTexture[0]->Texture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, job->SpecularTexture[0]->Texture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "SpecularUVRepeat"), 1, glm::value_ptr(job->SpecularTexture[0]->UVRepeat));
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_GreyTexture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "SpecularUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
		}

		glActiveTexture(GL_TEXTURE3);
		if (job->IncandescenceTexture.size() > 0 && job->IncandescenceTexture[0]->Texture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, job->IncandescenceTexture[0]->Texture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "GlowUVRepeat"), 1, glm::value_ptr(job->IncandescenceTexture[0]->UVRepeat));
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_BlackTexture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "GlowUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
		}
		break;
	}
	case RawModel::MaterialType::SplatMapping:
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, job->SplatMap->Texture->m_Texture);

		int texturePosition = GL_TEXTURE1;

		//Bind 5 diffuse textures
		std::string UniformName = "DiffuseUVRepeat";
		for (unsigned int i = 0; i < 5; i++) {
			glActiveTexture(texturePosition);
			if (job->DiffuseTexture.size() > i && job->DiffuseTexture[i]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->DiffuseTexture[i]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(job->DiffuseTexture[i]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}
			texturePosition++;
		}

		//Bind 5 Normal textures
		UniformName = "NormalUVRepeat";
		for (unsigned int i = 0; i < 5; i++) {
			glActiveTexture(texturePosition);
			if (job->NormalTexture.size() > i && job->NormalTexture[i]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->NormalTexture[i]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(job->NormalTexture[i]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_NeutralNormalTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}
			texturePosition++;
		}

		//Bind 5 Specular textures
		UniformName = "SpecularUVRepeat";
		for (unsigned int i = 0; i < 5; i++) {
			glActiveTexture(texturePosition);
			if (job->SpecularTexture.size() > i && job->SpecularTexture[i]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->SpecularTexture[i]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(job->SpecularTexture[i]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_GreyTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}
			texturePosition++;
		}

		//Bind 5 Incandescence textures
		UniformName = "GlowUVRepeat";
		for (unsigned int i = 0; i < 5; i++) {
			glActiveTexture(texturePosition);
			if (job->IncandescenceTexture.size() > i && job->IncandescenceTexture[i]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->IncandescenceTexture[i]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(job->IncandescenceTexture[i]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_BlackTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}
			texturePosition++;
		}
		break;
	}
	}
}

void DrawFinalPass::BindModelTextures(GLuint shaderHandle, std::shared_ptr<ModelJob>& job)
{
	switch (job->Type) {
		case RawModel::MaterialType::SingleTextures:
		case RawModel::MaterialType::Basic:
		{
			glActiveTexture(GL_TEXTURE0);
			if (job->DiffuseTexture.size() > 0 && job->DiffuseTexture[0]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->DiffuseTexture[0]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(job->DiffuseTexture[0]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}

			glActiveTexture(GL_TEXTURE1);
			if (job->NormalTexture.size() > 0 && job->NormalTexture[0]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->NormalTexture[0]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "NormalUVRepeat"), 1, glm::value_ptr(job->NormalTexture[0]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_NeutralNormalTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "NormalUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}

			glActiveTexture(GL_TEXTURE2);
			if (job->SpecularTexture.size() > 0 && job->SpecularTexture[0]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->SpecularTexture[0]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "SpecularUVRepeat"), 1, glm::value_ptr(job->SpecularTexture[0]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_GreyTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "SpecularUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}

			glActiveTexture(GL_TEXTURE3);
			if (job->IncandescenceTexture.size() > 0 && job->IncandescenceTexture[0]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->IncandescenceTexture[0]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "GlowUVRepeat"), 1, glm::value_ptr(job->IncandescenceTexture[0]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_BlackTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "GlowUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}
			break;
		}
		case RawModel::MaterialType::SplatMapping:
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, job->SplatMap->Texture->m_Texture);

			int texturePosition = GL_TEXTURE1;
		
			//Bind 5 diffuse textures
			std::string UniformName = "DiffuseUVRepeat";
			for (unsigned int i = 0; i < 3; i++) {
				glActiveTexture(texturePosition);
				if (job->DiffuseTexture.size() > i && job->DiffuseTexture[i]->Texture != nullptr) {
					glBindTexture(GL_TEXTURE_2D, job->DiffuseTexture[i]->Texture->m_Texture);
					glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(job->DiffuseTexture[i]->UVRepeat));
				} else {
					glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
					glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
				}
				texturePosition++;
			}

			//Bind 5 Normal textures
			UniformName = "NormalUVRepeat";
			for (unsigned int i = 0; i < 3; i++) {
				glActiveTexture(texturePosition);
				if (job->NormalTexture.size() > i && job->NormalTexture[i]->Texture != nullptr) {
					glBindTexture(GL_TEXTURE_2D, job->NormalTexture[i]->Texture->m_Texture);
					glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(job->NormalTexture[i]->UVRepeat));
				} else {
					glBindTexture(GL_TEXTURE_2D, m_NeutralNormalTexture->m_Texture);
					glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
				}
				texturePosition++;
			}

			//Bind 5 Specular textures
			UniformName = "SpecularUVRepeat";
			for (unsigned int i = 0; i < 3; i++) {
				glActiveTexture(texturePosition);
				if (job->SpecularTexture.size() > i && job->SpecularTexture[i]->Texture != nullptr) {
					glBindTexture(GL_TEXTURE_2D, job->SpecularTexture[i]->Texture->m_Texture);
					glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(job->SpecularTexture[i]->UVRepeat));
				} else {
					glBindTexture(GL_TEXTURE_2D, m_GreyTexture->m_Texture);
					glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
				}
				texturePosition++;
			}

			//Bind 5 Incandescence textures
			UniformName = "GlowUVRepeat";
			for (unsigned int i = 0; i < 3; i++) {
				glActiveTexture(texturePosition);
				if (job->IncandescenceTexture.size() > i && job->IncandescenceTexture[i]->Texture != nullptr) {
					glBindTexture(GL_TEXTURE_2D, job->IncandescenceTexture[i]->Texture->m_Texture);
					glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(job->IncandescenceTexture[i]->UVRepeat));
				} else {
					glBindTexture(GL_TEXTURE_2D, m_BlackTexture->m_Texture);
					glUniform2fv(glGetUniformLocation(shaderHandle, std::string(UniformName + ((char)(i + '1'))).c_str()), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
				}
				texturePosition++;
			}
			break;
		}
	}
}

