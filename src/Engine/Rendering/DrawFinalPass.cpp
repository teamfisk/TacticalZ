#include "Rendering/DrawFinalPass.h"
DrawFinalPass::DrawFinalPass(IRenderer* renderer, LightCullingPass* lightCullingPass, CubeMapPass* cubeMapPass, SSAOPass* ssaoPass)
	: m_Renderer(renderer)
	, m_LightCullingPass(lightCullingPass)
	, m_CubeMapPass(cubeMapPass)
	, m_SSAOPass(ssaoPass)
{
    //TODO: Make sure that uniforms are not sent into shader if not needed.
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
	CommonFunctions::GenerateTexture(&m_SceneTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewPortSize().Width, m_Renderer->GetViewPortSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
	CommonFunctions::GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    //GenerateMipMapTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, glm::vec2(m_Renderer->GetViewPortSize().Width, m_Renderer->GetViewPortSize().Height), GL_RGB16F, GL_FLOAT, 4);
    //GenerateTexture(&m_StencilTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_STENCIL, GL_STENCIL_INDEX8, GL_INT);

	CommonFunctions::GenerateTexture(&m_DepthBuffer, GL_CLAMP_TO_BORDER, GL_NEAREST,
		glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);

    m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_DepthBuffer, GL_DEPTH_STENCIL_ATTACHMENT)));
    //m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_StencilTexture, GL_STENCIL_ATTACHMENT)));
    m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_SceneTexture, GL_COLOR_ATTACHMENT0)));
    m_FinalPassFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_BloomTexture, GL_COLOR_ATTACHMENT1)));
    m_FinalPassFrameBuffer.Generate();
    GLERROR("FBO generation");

	CommonFunctions::GenerateTexture(&m_ShieldBuffer, GL_CLAMP_TO_BORDER, GL_NEAREST,
		glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
	m_ShieldDepthFrameBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_ShieldBuffer, GL_DEPTH_ATTACHMENT)));
	m_ShieldDepthFrameBuffer.Generate();
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

    m_FillDepthStencilBufferProgram = ResourceManager::Load<ShaderProgram>("#FillDepthBufferProgram");
    m_FillDepthStencilBufferProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/FillDepthBuffer.vert.glsl")));
    m_FillDepthStencilBufferProgram->Compile();
    m_FillDepthStencilBufferProgram->Link();
    GLERROR("Creating DepthFill program");

    m_FillDepthStencilBufferSkinnedProgram = ResourceManager::Load<ShaderProgram>("#FillDepthBufferProgramSkinned");
    m_FillDepthStencilBufferSkinnedProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/FillDepthBufferSkinned.vert.glsl")));
    m_FillDepthStencilBufferSkinnedProgram->Compile();
    m_FillDepthStencilBufferSkinnedProgram->Link();
    GLERROR("Creating DepthFill program");





	m_ForwardPlusShieldCheckProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusShieldCheckProgram");
	m_ForwardPlusShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
	m_ForwardPlusShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusShieldCheck.frag.glsl")));
	m_ForwardPlusShieldCheckProgram->Compile();
	m_ForwardPlusShieldCheckProgram->BindFragDataLocation(0, "sceneColor");
	m_ForwardPlusShieldCheckProgram->BindFragDataLocation(1, "bloomColor");
	m_ForwardPlusShieldCheckProgram->Link();
	GLERROR("Creating forward+ program");

	m_ExplosionEffectShieldCheckProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectShieldCheckProgram");
	m_ExplosionEffectShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
	m_ExplosionEffectShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
	m_ExplosionEffectShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusShieldCheck.frag.glsl")));
	m_ExplosionEffectShieldCheckProgram->Compile();
	m_ExplosionEffectShieldCheckProgram->BindFragDataLocation(0, "sceneColor");
	m_ExplosionEffectShieldCheckProgram->BindFragDataLocation(1, "bloomColor");
	m_ExplosionEffectShieldCheckProgram->Link();
	GLERROR("Creating explosion program");

	m_SpriteShieldCheckProgram = ResourceManager::Load<ShaderProgram>("#SpriteShieldCheckProgram");
	m_SpriteShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Sprite.vert.glsl")));
	m_SpriteShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/SpriteShieldCheck.frag.glsl")));
	m_SpriteShieldCheckProgram->Compile();
	m_SpriteShieldCheckProgram->BindFragDataLocation(0, "sceneColor");
	m_SpriteShieldCheckProgram->BindFragDataLocation(1, "bloomColor");
	m_SpriteShieldCheckProgram->Link();
	GLERROR("Creating sprite program");

	m_ForwardPlusSplatMapShieldCheckProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusSplatMapShieldCheckProgram");
	m_ForwardPlusSplatMapShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
	m_ForwardPlusSplatMapShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusSplatMapRGBShieldCheck.frag.glsl")));
	m_ForwardPlusSplatMapShieldCheckProgram->Compile();
	m_ForwardPlusSplatMapShieldCheckProgram->BindFragDataLocation(0, "sceneColor");
	m_ForwardPlusSplatMapShieldCheckProgram->BindFragDataLocation(1, "bloomColor");
	m_ForwardPlusSplatMapShieldCheckProgram->Link();
	GLERROR("Creating Forward SplatMap program");

	m_ExplosionEffectSplatMapShieldCheckProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectSplatMapShieldCheckProgram");
	m_ExplosionEffectSplatMapShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlus.vert.glsl")));
	m_ExplosionEffectSplatMapShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
	m_ExplosionEffectSplatMapShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusSplatMapRGBShieldCheck.frag.glsl")));
	m_ExplosionEffectSplatMapShieldCheckProgram->Compile();
	m_ExplosionEffectSplatMapShieldCheckProgram->BindFragDataLocation(0, "sceneColor");
	m_ExplosionEffectSplatMapShieldCheckProgram->BindFragDataLocation(1, "bloomColor");
	m_ExplosionEffectSplatMapShieldCheckProgram->Link();
	GLERROR("Creating explosion SplatMap program");

	m_ForwardPlusSkinnedShieldCheckProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusSkinnedShieldCheckProgram");
	m_ForwardPlusSkinnedShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlusSkinned.vert.glsl")));
	m_ForwardPlusSkinnedShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusShieldCheck.frag.glsl")));
	m_ForwardPlusSkinnedShieldCheckProgram->Compile();
	m_ForwardPlusSkinnedShieldCheckProgram->BindFragDataLocation(0, "sceneColor");
	m_ForwardPlusSkinnedShieldCheckProgram->BindFragDataLocation(1, "bloomColor");
	m_ForwardPlusSkinnedShieldCheckProgram->Link();
	GLERROR("Creating forward+ Skinned program");

	m_ExplosionEffectSkinnedShieldCheckProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectSkinnedShieldCheckProgram");
	m_ExplosionEffectSkinnedShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlusSkinned.vert.glsl")));
	m_ExplosionEffectSkinnedShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
	m_ExplosionEffectSkinnedShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusShieldCheck.frag.glsl")));
	m_ExplosionEffectSkinnedShieldCheckProgram->Compile();
	m_ExplosionEffectSkinnedShieldCheckProgram->BindFragDataLocation(0, "sceneColor");
	m_ExplosionEffectSkinnedShieldCheckProgram->BindFragDataLocation(1, "bloomColor");
	m_ExplosionEffectSkinnedShieldCheckProgram->Link();
	GLERROR("Creating explosion Skinned program");

	m_ExplosionEffectSplatMapSkinnedShieldCheckProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectSplatMapSkinnedShieldCheckProgram");
	m_ExplosionEffectSplatMapSkinnedShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlusSkinned.vert.glsl")));
	m_ExplosionEffectSplatMapSkinnedShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusSplatMapRGBShieldCheck.frag.glsl")));
	m_ExplosionEffectSplatMapSkinnedShieldCheckProgram->Compile();
	m_ExplosionEffectSplatMapSkinnedShieldCheckProgram->BindFragDataLocation(0, "sceneColor");
	m_ExplosionEffectSplatMapSkinnedShieldCheckProgram->BindFragDataLocation(1, "bloomColor");
	m_ExplosionEffectSplatMapSkinnedShieldCheckProgram->Link();
	GLERROR("Creating Forward SplatMap Skinned program");

	m_ForwardPlusSplatMapSkinnedShieldCheckProgram = ResourceManager::Load<ShaderProgram>("#ForwardPlusSplatMapSkinnedShieldCheckProgram");
	m_ForwardPlusSplatMapSkinnedShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ForwardPlusSkinned.vert.glsl")));
	m_ForwardPlusSplatMapSkinnedShieldCheckProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ForwardPlusSplatMapRGBShieldCheck.frag.glsl")));
	m_ForwardPlusSplatMapSkinnedShieldCheckProgram->Compile();
	m_ForwardPlusSplatMapSkinnedShieldCheckProgram->BindFragDataLocation(0, "sceneColor");
	m_ForwardPlusSplatMapSkinnedShieldCheckProgram->BindFragDataLocation(1, "bloomColor");
	m_ForwardPlusSplatMapSkinnedShieldCheckProgram->Link();
	GLERROR("Creating Forward SplatMap Skinned program");
	
}

void DrawFinalPass::Draw(RenderScene& scene)
{
    GLERROR("Pre");
	DrawFinalPassState* stateDethp = new DrawFinalPassState(m_ShieldDepthFrameBuffer.GetHandle());
	//Draw shields to stencil
	DrawToDepthStencilBuffer(scene.Jobs.ShieldObjects, scene);
	GLERROR("StencilPass");
	delete stateDethp;


	DrawFinalPassState* state = new DrawFinalPassState(m_FinalPassFrameBuffer.GetHandle());
    if (scene.ClearDepth) {
        //glClear(GL_DEPTH_BUFFER_BIT);
		state->Disable(GL_DEPTH_TEST);
		state->DepthMask(GL_FALSE);
    }
    //TODO: Do we need check for this or will it be per scene always?
    glClearStencil(0x00);
    glClear(GL_STENCIL_BUFFER_BIT);

    //Fill depth buffer
	state->Enable(GL_STENCIL_TEST);
	state->StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	state->StencilFunc(GL_ALWAYS, 1, 0xFF);
	state->StencilMask(0xFF);
	state->DepthMask(GL_FALSE);
	//DrawToDepthStencilBuffer(scene.Jobs.ShieldObjects, scene);
	state->DepthMask(GL_TRUE);

    //Draw Opaque shielded objects
	state->Disable(GL_STENCIL_TEST);
    state->StencilFunc(GL_NOTEQUAL, 1, 0xFF);
    state->StencilMask(0x00);
	DrawModelRenderQueuesWithShieldCheck(scene.Jobs.OpaqueShieldedObjects, scene); //might need changing
    GLERROR("Shielded Opaque object");

	//Draw Opaque objects
	//state->StencilMask(0x00);
	DrawModelRenderQueues(scene.Jobs.OpaqueObjects, scene);
	GLERROR("OpaqueObjects");

	//state->Disable(GL_STENCIL_TEST);
    //Draw Transparen Shielded objects
	state->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DrawModelRenderQueuesWithShieldCheck(scene.Jobs.TransparentObjects, scene); //might need changing
    GLERROR("Shielded Transparent objects");

	//Draw Transparen objects
	//state->BlendFunc(GL_ONE, GL_ONE);
	//state->StencilFunc(GL_EQUAL, 1, 0xFF);
	//DrawModelRenderQueues(scene.Jobs.TransparentObjects, scene);
	GLERROR("TransparentObjects");
	//state->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DrawSprites(scene.Jobs.SpriteJob, scene);
	GLERROR("SpriteJobs");

	delete state;
    GLERROR("END");
    
}


void DrawFinalPass::ClearBuffer()
{
    GLERROR("PRE");
	m_ShieldDepthFrameBuffer.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	m_ShieldDepthFrameBuffer.Unbind();
    m_FinalPassFrameBuffer.Bind();
    GLERROR("Bind HighRes");
    glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    glScissor(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    GLERROR("ViewPort,Scissor LowRes");
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_FinalPassFrameBuffer.Unbind();
    GLERROR("END");
}


void DrawFinalPass::OnWindowResize()
{
    //InitializeFrameBuffers();
	CommonFunctions::GenerateTexture(&m_DepthBuffer, GL_CLAMP_TO_BORDER, GL_NEAREST,
		glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    CommonFunctions::GenerateTexture(&m_SceneTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
	CommonFunctions::GenerateTexture(&m_BloomTexture, GL_CLAMP_TO_EDGE, GL_LINEAR, glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RGB16F, GL_RGB, GL_FLOAT);
    m_FinalPassFrameBuffer.Generate();

    GLERROR("Error changing texture resolutions");
}

void DrawFinalPass::DrawModelRenderQueues(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene)
{
    GLuint forwardHandle = m_ForwardPlusProgram->GetHandle();
	GLuint explosionHandle = m_ExplosionEffectProgram->GetHandle();
	GLuint explosionSplatMapHandle = m_ExplosionEffectSplatMapProgram->GetHandle();
	GLuint forwardSplatMapHandle = m_ForwardPlusSplatMapProgram->GetHandle();
	GLuint forwardSkinnedHandle = m_ForwardPlusSkinnedProgram->GetHandle();
	GLuint explosionSkinnedHandle = m_ExplosionEffectSkinnedProgram->GetHandle();
	GLuint explosionSplatMapSkinnedHandle = m_ExplosionEffectSplatMapSkinnedProgram->GetHandle();
	GLuint forwardSplatMapSkinnedHandle = m_ForwardPlusSplatMapSkinnedProgram->GetHandle();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightCullingPass->LightSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightCullingPass->LightGridSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightCullingPass->LightIndexSSBO());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SSAOPass->SSAOTexture());

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
						glActiveTexture(GL_TEXTURE5);
						glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
						glUniform3fv(glGetUniformLocation(explosionSkinnedHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));

						std::vector<glm::mat4> frameBones;
						if (explosionEffectJob->AnimationOffset.animation != nullptr) {
							frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations, explosionEffectJob->AnimationOffset);
						}
						else {
							frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations);
						}
						glUniformMatrix4fv(glGetUniformLocation(explosionSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
					}
					else {
						m_ExplosionEffectProgram->Bind();
						GLERROR("Bind ExplosionEffect program");
						//bind uniforms
						BindExplosionUniforms(explosionHandle, explosionEffectJob, scene);
						//bind textures
						BindExplosionTextures(explosionHandle, explosionEffectJob);
						glActiveTexture(GL_TEXTURE5);
						glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
						glUniform3fv(glGetUniformLocation(explosionHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));
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
						if (explosionEffectJob->AnimationOffset.animation != nullptr) {
							frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations, explosionEffectJob->AnimationOffset);
						}
						else {
							frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations);
						}
						glUniformMatrix4fv(glGetUniformLocation(explosionSplatMapSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
					}
					else {
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
							GLERROR("Basic/SingleTextures BindModelUniforms");
						//bind textures
						BindModelTextures(forwardSkinnedHandle, modelJob);
							GLERROR("Basic/SingleTextures BindModelTextures");
						glActiveTexture(GL_TEXTURE5);
							GLERROR("Basic/SingleTextures CameraPosition1");
						glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
							GLERROR("Basic/SingleTextures CameraPosition2");
							glUniform3fv(glGetUniformLocation(forwardSkinnedHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));
							GLERROR("Basic/SingleTextures CameraPosition3");
						std::vector<glm::mat4> frameBones;
						if (modelJob->AnimationOffset.animation != nullptr) {
							frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations, modelJob->AnimationOffset);
						}
						else {
							frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations);
						}
						glUniformMatrix4fv(glGetUniformLocation(forwardSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
							GLERROR("Basic/SingleTextures skinned end");
					}
					else {
						m_ForwardPlusProgram->Bind();
						GLERROR("Bind ForwardPlusProgram");
						//bind uniforms
						BindModelUniforms(forwardHandle, modelJob, scene);
						//bind textures
						BindModelTextures(forwardHandle, modelJob);
						glActiveTexture(GL_TEXTURE5);
						glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
						glUniform3fv(glGetUniformLocation(forwardHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));
							GLERROR("Basic/SingleTextures end");
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
						if (modelJob->AnimationOffset.animation != nullptr) {
							frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations, modelJob->AnimationOffset);
						}
						else {
							frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations);
						}
						glUniformMatrix4fv(glGetUniformLocation(forwardSplatMapSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
							GLERROR("SplatMapping skinned end");
					}
					else {
						m_ForwardPlusSplatMapProgram->Bind();
						GLERROR("Bind SplatMap program");
						//bind uniforms
						BindModelUniforms(forwardSplatMapHandle, modelJob, scene);
						//bind textures
						BindModelTextures(forwardSplatMapHandle, modelJob);
						GLERROR("SplatMapping end");
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

void DrawFinalPass::DrawModelRenderQueuesWithShieldCheck(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene)
{
	GLuint forwardHandle = m_ForwardPlusProgram->GetHandle();
	GLuint explosionHandle = m_ExplosionEffectProgram->GetHandle();
	GLuint explosionSplatMapHandle = m_ExplosionEffectSplatMapProgram->GetHandle();
	GLuint forwardSplatMapHandle = m_ForwardPlusSplatMapProgram->GetHandle();
	GLuint forwardSkinnedHandle = m_ForwardPlusSkinnedProgram->GetHandle();
	GLuint explosionSkinnedHandle = m_ExplosionEffectSkinnedProgram->GetHandle();
	GLuint explosionSplatMapSkinnedHandle = m_ExplosionEffectSplatMapSkinnedProgram->GetHandle();
	GLuint forwardSplatMapSkinnedHandle = m_ForwardPlusSplatMapSkinnedProgram->GetHandle();

	GLuint forwardShieldCheckHandle = m_ForwardPlusShieldCheckProgram->GetHandle();
	GLuint explosionShieldCheckHandle = m_ExplosionEffectShieldCheckProgram->GetHandle();
	GLuint explosionSplatMapShieldCheckHandle = m_ExplosionEffectSplatMapShieldCheckProgram->GetHandle();
	GLuint forwardSplatShieldCheckHandle = m_ForwardPlusSplatMapShieldCheckProgram->GetHandle();
	GLuint forwardSkinnedShieldCheckHandle = m_ForwardPlusSkinnedShieldCheckProgram->GetHandle();
	GLuint explosionSkinnedShieldCheckHandle = m_ExplosionEffectSkinnedShieldCheckProgram->GetHandle();
	GLuint explosionSplatMapSkinnedShieldCheckHandle = m_ExplosionEffectSplatMapSkinnedShieldCheckProgram->GetHandle();
	GLuint forwardSplatMapSkinnedShieldCheckHandle = m_ForwardPlusSplatMapSkinnedShieldCheckProgram->GetHandle();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightCullingPass->LightSSBO());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightCullingPass->LightGridSSBO());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightCullingPass->LightIndexSSBO());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SSAOPass->SSAOTexture());

	glActiveTexture(GL_TEXTURE31);
	glBindTexture(GL_TEXTURE_2D, m_ShieldBuffer);

	for (auto &job : jobs) {
		auto explosionEffectJob = std::dynamic_pointer_cast<ExplosionEffectJob>(job);
		if (explosionEffectJob) {
			if (explosionEffectJob->IsShielded) {
				switch (explosionEffectJob->Type) {
				case RawModel::MaterialType::Basic:
				case RawModel::MaterialType::SingleTextures:
					{
						if (explosionEffectJob->Model->IsSkinned()) {

							m_ExplosionEffectSkinnedShieldCheckProgram->Bind();
							GLERROR("Bind ExplosionEffectSkinned program");
							//bind uniforms
							BindExplosionUniforms(explosionSkinnedShieldCheckHandle, explosionEffectJob, scene);
							//bind textures
							BindExplosionTextures(explosionSkinnedShieldCheckHandle, explosionEffectJob);
							glActiveTexture(GL_TEXTURE5);
							glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
							glUniform3fv(glGetUniformLocation(explosionSkinnedShieldCheckHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));

							std::vector<glm::mat4> frameBones;
							if (explosionEffectJob->AnimationOffset.animation != nullptr) {
								frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations, explosionEffectJob->AnimationOffset);
							}
							else {
								frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations);
							}
							glUniformMatrix4fv(glGetUniformLocation(explosionSkinnedShieldCheckHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
						}
						else {
							m_ExplosionEffectShieldCheckProgram->Bind();
							GLERROR("Bind ExplosionEffect program");
							//bind uniforms
							BindExplosionUniforms(explosionShieldCheckHandle, explosionEffectJob, scene);
							//bind textures
							BindExplosionTextures(explosionShieldCheckHandle, explosionEffectJob);
							glActiveTexture(GL_TEXTURE5);
							glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
							glUniform3fv(glGetUniformLocation(explosionShieldCheckHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));

						}
						break;
					}
				case RawModel::MaterialType::SplatMapping:
					{
						if (explosionEffectJob->Model->IsSkinned()) {
							m_ExplosionEffectSplatMapSkinnedShieldCheckProgram->Bind();
							GLERROR("Bind ExplosionEffectSplatMapSkinned program");
							//bind uniforms
							BindExplosionUniforms(explosionSplatMapSkinnedShieldCheckHandle, explosionEffectJob, scene);
							//bind textures
							BindExplosionTextures(explosionSplatMapSkinnedShieldCheckHandle, explosionEffectJob);
							GLERROR("asdasd");
							std::vector<glm::mat4> frameBones;
							if (explosionEffectJob->AnimationOffset.animation != nullptr) {
								frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations, explosionEffectJob->AnimationOffset);
							}
							else {
								frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations);
							}
							glUniformMatrix4fv(glGetUniformLocation(explosionSplatMapSkinnedShieldCheckHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

						}
						else {
							m_ExplosionEffectSplatMapShieldCheckProgram->Bind();
							GLERROR("Bind ExplosionEffectSplatMap program");
							//bind uniforms
							//bind uniforms
							BindExplosionUniforms(explosionSplatMapShieldCheckHandle, explosionEffectJob, scene);
							//bind textures
							BindExplosionTextures(explosionSplatMapShieldCheckHandle, explosionEffectJob);
							GLERROR("asdasd");
						}
						break;
					}
				}
			} else {
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
							glActiveTexture(GL_TEXTURE5);
							glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
							glUniform3fv(glGetUniformLocation(explosionSkinnedHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));

							std::vector<glm::mat4> frameBones;
							if (explosionEffectJob->AnimationOffset.animation != nullptr) {
								frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations, explosionEffectJob->AnimationOffset);
							}
							else {
								frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations);
							}
							glUniformMatrix4fv(glGetUniformLocation(explosionSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
						}
						else {
							m_ExplosionEffectProgram->Bind();
							GLERROR("Bind ExplosionEffect program");
							//bind uniforms
							BindExplosionUniforms(explosionHandle, explosionEffectJob, scene);
							//bind textures
							BindExplosionTextures(explosionHandle, explosionEffectJob);
							glActiveTexture(GL_TEXTURE5);
							glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
							glUniform3fv(glGetUniformLocation(explosionHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));
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
							if (explosionEffectJob->AnimationOffset.animation != nullptr) {
								frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations, explosionEffectJob->AnimationOffset);
							}
							else {
								frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations);
							}
							glUniformMatrix4fv(glGetUniformLocation(explosionSplatMapSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
						}
						else {
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
				if (modelJob->IsShielded) {
					switch (modelJob->Type) {
					case RawModel::MaterialType::Basic:
					case RawModel::MaterialType::SingleTextures:
						{
							if (modelJob->Model->IsSkinned()) {
								m_ForwardPlusSkinnedShieldCheckProgram->Bind();
								GLERROR("Bind ForwardPlusSkinnedProgram");
								//bind uniforms
								BindModelUniforms(forwardSkinnedShieldCheckHandle, modelJob, scene);
								//bind textures
								BindModelTextures(forwardSkinnedShieldCheckHandle, modelJob);
								glActiveTexture(GL_TEXTURE5);
								glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
								glUniform3fv(glGetUniformLocation(forwardSkinnedShieldCheckHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));

								std::vector<glm::mat4> frameBones;
								if (modelJob->AnimationOffset.animation != nullptr) {
									frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations, modelJob->AnimationOffset);
								}
								else {
									frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations);
								}
								glUniformMatrix4fv(glGetUniformLocation(forwardSkinnedShieldCheckHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

							}
							else {
								m_ForwardPlusShieldCheckProgram->Bind();
								GLERROR("Bind ForwardPlusProgram");
								//bind uniforms
								BindModelUniforms(forwardShieldCheckHandle, modelJob, scene);
								//bind textures
								BindModelTextures(forwardShieldCheckHandle, modelJob);
								glActiveTexture(GL_TEXTURE5);
								glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
								glUniform3fv(glGetUniformLocation(forwardShieldCheckHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));
							}
							break;
						}
					case RawModel::MaterialType::SplatMapping:
						{
							if (modelJob->Model->IsSkinned()) {
								m_ForwardPlusSplatMapSkinnedShieldCheckProgram->Bind();
								GLERROR("Bind SplatMap program");
								//bind uniforms
								BindModelUniforms(forwardSplatMapSkinnedShieldCheckHandle, modelJob, scene);
								//bind textures
								BindModelTextures(forwardSplatMapSkinnedShieldCheckHandle, modelJob);
								GLERROR("asdasd");
								std::vector<glm::mat4> frameBones;
								if (modelJob->AnimationOffset.animation != nullptr) {
									frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations, modelJob->AnimationOffset);
								}
								else {
									frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations);
								}
								glUniformMatrix4fv(glGetUniformLocation(forwardSplatMapSkinnedShieldCheckHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

							}
							else {
								m_ForwardPlusSplatMapShieldCheckProgram->Bind();
								GLERROR("Bind SplatMap program");
								//bind uniforms
								BindModelUniforms(forwardSplatShieldCheckHandle, modelJob, scene);
								//bind textures
								BindModelTextures(forwardSplatShieldCheckHandle, modelJob);
								GLERROR("asdasd");
							}
							break;
						}
					}
				} else {
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
								glActiveTexture(GL_TEXTURE5);
								glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
								glUniform3fv(glGetUniformLocation(forwardSkinnedHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));

								std::vector<glm::mat4> frameBones;
								if (modelJob->AnimationOffset.animation != nullptr) {
									frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations, modelJob->AnimationOffset);
								}
								else {
									frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations);
								}
								glUniformMatrix4fv(glGetUniformLocation(forwardSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

							}
							else {
								m_ForwardPlusProgram->Bind();
								GLERROR("Bind ForwardPlusProgram");
								//bind uniforms
								BindModelUniforms(forwardHandle, modelJob, scene);
								//bind textures
								BindModelTextures(forwardHandle, modelJob);
								glActiveTexture(GL_TEXTURE5);
								glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapPass->m_CubeMapTexture);
								glUniform3fv(glGetUniformLocation(forwardHandle, "CameraPosition"), 1, glm::value_ptr(scene.Camera->Position()));
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
								if (modelJob->AnimationOffset.animation != nullptr) {
									frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations, modelJob->AnimationOffset);
								}
								else {
									frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations);
								}
								glUniformMatrix4fv(glGetUniformLocation(forwardSplatMapSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

							}
							else {
								m_ForwardPlusSplatMapProgram->Bind();
								GLERROR("Bind SplatMap program");
								//bind uniforms
								BindModelUniforms(forwardSplatMapHandle, modelJob, scene);
								//bind textures
								BindModelTextures(forwardSplatMapHandle, modelJob);
								GLERROR("asdasd");
							}
							break;
						}
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
            if (explosionEffectJob->AnimationOffset.animation != nullptr) {
                frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations, explosionEffectJob->AnimationOffset);
            } else {
                frameBones = explosionEffectJob->Skeleton->GetFrameBones(explosionEffectJob->Animations);
            }
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
                if (modelJob->AnimationOffset.animation != nullptr) {
                    frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations, modelJob->AnimationOffset);
                } else {
                    frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations);
                }
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


void DrawFinalPass::DrawToDepthStencilBuffer(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene)
{
    
   
    for (auto &job : jobs) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
      
        if(modelJob->Model->IsSkinned()) {
            m_FillDepthStencilBufferSkinnedProgram->Bind();
            GLuint shaderHandle = m_FillDepthStencilBufferSkinnedProgram->GetHandle();
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));

            std::vector<glm::mat4> frameBones;
            if (modelJob->AnimationOffset.animation != nullptr) {
                frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations, modelJob->AnimationOffset);
            } else {
                frameBones = modelJob->Skeleton->GetFrameBones(modelJob->Animations);
            }
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

        } else {
            m_FillDepthStencilBufferProgram->Bind();
            GLuint shaderHandle = m_FillDepthStencilBufferProgram->GetHandle();
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

            glActiveTexture(GL_TEXTURE1);
            if (spriteJob->DiffuseTexture != nullptr) {
                glBindTexture(GL_TEXTURE_2D, spriteJob->DiffuseTexture->m_Texture);
            } else {
                glBindTexture(GL_TEXTURE_2D, m_ErrorTexture->m_Texture);
            }

            glActiveTexture(GL_TEXTURE2);
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
	glUniform1i(glGetUniformLocation(shaderHandle, "SSAOQuality"), m_SSAOPass->TextureQuality());
	GLERROR("Bind 1 uniform");
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(job->Matrix));
	GLERROR("Bind 2 uniform");
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
	GLERROR("Bind 3 uniform");
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
	GLERROR("Bind 4 uniform");

    glUniform2f(glGetUniformLocation(shaderHandle, "ScreenDimensions"), m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
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
    GLERROR("Bind 20 uniform");
    glUniform1f(glGetUniformLocation(shaderHandle, "GlowIntensity"), job->GlowIntensity);
    GLERROR("END");
}

void DrawFinalPass::BindModelUniforms(GLuint shaderHandle, std::shared_ptr<ModelJob>& job, RenderScene& scene)
{
	glUniform1i(glGetUniformLocation(shaderHandle, "SSAOQuality"), m_SSAOPass->TextureQuality());
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
	glUniform2f(Location_ScreenDimensions, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
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

    GLERROR("Bind 10 uniform");
    GLint Location_GlowIntensity = glGetUniformLocation(shaderHandle, "GlowIntensity");

    glUniform1f(Location_GlowIntensity, job->GlowIntensity);

	GLERROR("END");
}

void DrawFinalPass::BindExplosionTextures(GLuint shaderHandle, std::shared_ptr<ExplosionEffectJob>& job)
{


	switch (job->Type) {
	case RawModel::MaterialType::SingleTextures:
	case RawModel::MaterialType::Basic:
	{
		glActiveTexture(GL_TEXTURE1);
		if (job->DiffuseTexture.size() > 0 && job->DiffuseTexture[0]->Texture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, job->DiffuseTexture[0]->Texture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(job->DiffuseTexture[0]->UVRepeat));
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
		}

		glActiveTexture(GL_TEXTURE2);
		if (job->NormalTexture.size() > 0 && job->NormalTexture[0]->Texture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, job->NormalTexture[0]->Texture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "NormalUVRepeat"), 1, glm::value_ptr(job->NormalTexture[0]->UVRepeat));
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_NeutralNormalTexture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "NormalUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
		}

		glActiveTexture(GL_TEXTURE3);
		if (job->SpecularTexture.size() > 0 && job->SpecularTexture[0]->Texture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, job->SpecularTexture[0]->Texture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "SpecularUVRepeat"), 1, glm::value_ptr(job->SpecularTexture[0]->UVRepeat));
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_GreyTexture->m_Texture);
			glUniform2fv(glGetUniformLocation(shaderHandle, "SpecularUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
		}

		glActiveTexture(GL_TEXTURE4);
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
		glActiveTexture(GL_TEXTURE1);
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
			glActiveTexture(GL_TEXTURE1);
			if (job->DiffuseTexture.size() > 0 && job->DiffuseTexture[0]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->DiffuseTexture[0]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(job->DiffuseTexture[0]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}

			glActiveTexture(GL_TEXTURE2);
			if (job->NormalTexture.size() > 0 && job->NormalTexture[0]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->NormalTexture[0]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "NormalUVRepeat"), 1, glm::value_ptr(job->NormalTexture[0]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_NeutralNormalTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "NormalUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}

			glActiveTexture(GL_TEXTURE3);
			if (job->SpecularTexture.size() > 0 && job->SpecularTexture[0]->Texture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, job->SpecularTexture[0]->Texture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "SpecularUVRepeat"), 1, glm::value_ptr(job->SpecularTexture[0]->UVRepeat));
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_GreyTexture->m_Texture);
				glUniform2fv(glGetUniformLocation(shaderHandle, "SpecularUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
			}

			glActiveTexture(GL_TEXTURE4);
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
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, job->SplatMap->Texture->m_Texture);

			int texturePosition = GL_TEXTURE2;
		
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

