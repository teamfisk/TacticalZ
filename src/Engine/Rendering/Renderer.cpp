#include "Rendering/Renderer.h"

std::unordered_map<GLFWwindow*, Renderer*> Renderer::m_WindowToRenderer;

void Renderer::Initialize()
{
	m_SSAO_Quality = m_Config->Get<int>("SSAO.Quality", 0);
	m_GLOW_Quality = m_Config->Get<int>("GLOW.Quality", 0);
	InitializeWindow();
    
    InitializeRenderPasses();

	glfwSwapInterval(m_VSYNC);
	InitializeShaders();
    InitializeTextures();
    m_TextPass = new TextPass();
    m_TextPass->Initialize();

   /* m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.obj");
    m_UnitQuad = ResourceManager::Load<Model>(sModels/Core/UnitQuad.obj");
    m_UnitSphere = ResourceManager::Load<Model>("Models/Core/UnitSphere.obj");*/

    m_ImGuiRenderPass = new ImGuiRenderPass(this, m_EventBroker);
}

void Renderer::glfwFrameBufferCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Renderer* currentRenderer = m_WindowToRenderer[window];
    currentRenderer->m_ViewportSize = Rectangle(width, height);
	currentRenderer->m_PickingPass->OnWindowResize();
    currentRenderer->m_DrawFinalPass->OnWindowResize();
    currentRenderer->m_LightCullingPass->OnWindowResize();
    currentRenderer->m_DrawBloomPass->OnWindowResize();
	currentRenderer->m_SSAOPass->OnWindowResize();
}

void Renderer::InitializeWindow()
{
	// Initialize GLFW
	if (!glfwInit()) {
		LOG_ERROR("GLFW: Initialization failed");
		exit(EXIT_FAILURE);
	}

	// Create a window
	GLFWmonitor* monitor = nullptr;
	if (m_Fullscreen) {
		monitor = glfwGetPrimaryMonitor();
	}
	//glfwWindowHint(GLFW_SAMPLES, 8);
	m_Window = glfwCreateWindow(m_Resolution.Width, m_Resolution.Height, "daydream", monitor, nullptr);
	if (!m_Window) {
		LOG_ERROR("GLFW: Failed to create window");
		exit(EXIT_FAILURE);
	}
    glfwSetFramebufferSizeCallback(m_Window, &glfwFrameBufferCallback);
	glfwMakeContextCurrent(m_Window);

	// GL version info
	glGetIntegerv(GL_MAJOR_VERSION, &m_GLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &m_GLVersion[1]);
	m_GLVendor = (GLchar*)glGetString(GL_VENDOR);
	std::stringstream ss;
	ss << m_GLVendor << " OpenGL " << m_GLVersion[0] << "." << m_GLVersion[1];
#ifdef DEBUG
	ss << " DEBUG";
#endif
	LOG_INFO(ss.str().c_str());
    SetWindowTitle(ss.str());

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		LOG_ERROR("GLEW: Initialization failed");
		exit(EXIT_FAILURE);
	}

    m_WindowToRenderer[m_Window] = this;

    int windowSize[2];
    glfwGetFramebufferSize(m_Window, &windowSize[0], &windowSize[1]);
    m_ViewportSize = Rectangle(windowSize[0], windowSize[1]);
}

void Renderer::InitializeShaders()
{
    m_BasicForwardProgram = ResourceManager::Load<ShaderProgram>("#m_BasicForwardProgram");
    m_ExplosionEffectProgram = ResourceManager::Load<ShaderProgram>("#ExplosionEffectProgram");
    //m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ExplosionEffect.vert.glsl")));
    //m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new GeometryShader("Shaders/ExplosionEffect.geom.glsl")));
    //m_ExplosionEffectProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/ExplosionEffect.frag.glsl")));
    //m_ExplosionEffectProgram->Compile();
    //m_ExplosionEffectProgram->Link();
}

void Renderer::InputUpdate(double dt)
{
   
}

void Renderer::Update(double dt)
{
    m_EventBroker->Process<Renderer>();
    InputUpdate(dt);
    m_TextPass->Update();
    m_ImGuiRenderPass->Update(dt);
}

void Renderer::Draw(RenderFrame& frame)
{
    GLERROR("PRE");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ImGui::Combo("Draw textures", &m_DebugTextureToDraw, "Final\0Scene\0Bloom\0Gaussian\0Picking\0Ambient Occlusion");
    ImGui::Combo("CubeMap", &m_CubeMapTexture, "Nevada(512)\0Sky(1024)");
    if(m_CubeMapTexture == 0) {
        m_CubeMapPass->LoadTextures("Nevada");
    } else if (m_CubeMapTexture == 1) {
        m_CubeMapPass->LoadTextures("Sky");
    }

	ImGui::SliderInt("SSAO Quality", &m_SSAO_Quality, 0, 3);
	ImGui::SliderInt("Glow Quality", &m_GLOW_Quality, 0, 3);
	m_SSAOPass->ChangeQuality(m_SSAO_Quality);
	m_DrawBloomPass->ChangeQuality(m_GLOW_Quality);
    GLERROR("SSAO Settings");
    //clear buffer 0
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Clear other buffers 
    PerformanceTimer::StartTimer("Renderer-ClearBuffers");
    m_PickingPass->ClearPicking();
    m_DrawFinalPass->ClearBuffer();
    m_DrawBloomPass->ClearBuffer();
	m_SSAOPass->ClearBuffer();
    PerformanceTimer::StopTimer("Renderer-ClearBuffers");
    GLERROR("ClearBuffers");
	for (auto scene : frame.RenderScenes) {
		PerformanceTimer::StartTimer("Renderer-PickingPass");
		m_PickingPass->Draw(*scene);
		GLERROR("Drawing pickingpass");
		PerformanceTimer::StopTimer("Renderer-PickingPass");
	}
	PerformanceTimer::StartTimer("Renderer-AO generation");
	m_SSAOPass->Draw(*m_PickingPass->DepthBuffer(), frame.RenderScenes.front()->Camera);
	PerformanceTimer::StopTimer("Renderer-AO generation");
    for (auto scene : frame.RenderScenes){
        PerformanceTimer::StartTimer("Renderer-Depth");
        SortRenderJobsByDepth(*scene);
        GLERROR("SortByDepth");
        PerformanceTimer::StartTimerAndStopPrevious("Renderer-Generate Frustrums");
        m_LightCullingPass->GenerateNewFrustum(*scene);
        GLERROR("Generate frustums");
        PerformanceTimer::StartTimerAndStopPrevious("Renderer-Filling Light List");
        m_LightCullingPass->FillLightList(*scene);
        GLERROR("Filling light list");
        PerformanceTimer::StartTimerAndStopPrevious("Renderer-Light Culling");
        m_LightCullingPass->CullLights(*scene);
        GLERROR("LightCulling");
		PerformanceTimer::StartTimerAndStopPrevious("Renderer-Draw Geometry+Light");
		m_DrawFinalPass->Draw(*scene);
        GLERROR("Draw Geometry+Light");
        //m_DrawScenePass->Draw(*scene);

        PerformanceTimer::StartTimerAndStopPrevious("Renderer-Draw Text");
        m_TextPass->Draw(*scene, *m_DrawFinalPass->FinalPassFrameBuffer());
        GLERROR("Draw Text");
        PerformanceTimer::StopTimer("Renderer-Draw Text");
    }

    PerformanceTimer::StartTimer("Renderer-Draw Bloom");
    m_DrawBloomPass->Draw(m_DrawFinalPass->BloomTexture());
    PerformanceTimer::StopTimer("Renderer-Draw Bloom");

    if (m_DebugTextureToDraw == 0) {
        PerformanceTimer::StartTimer("Renderer-Color Correction Pass");
        m_DrawColorCorrectionPass->Draw(m_DrawFinalPass->SceneTexture(), m_DrawBloomPass->GaussianTexture(), frame.Gamma, frame.Exposure);
        PerformanceTimer::StopTimer("Renderer-Color Correction Pass");
    }

    PerformanceTimer::StartTimer("Renderer-Misc Debug Draws");
    if (m_DebugTextureToDraw == 1) {
        m_DrawScreenQuadPass->Draw(m_DrawFinalPass->SceneTexture());
    }
    if (m_DebugTextureToDraw == 2) {
        m_DrawScreenQuadPass->Draw(m_DrawFinalPass->BloomTexture());
    }
    if (m_DebugTextureToDraw == 3) {
        m_DrawScreenQuadPass->Draw(m_DrawBloomPass->GaussianTexture());
    }
    if (m_DebugTextureToDraw == 4) {
        m_DrawScreenQuadPass->Draw(m_PickingPass->PickingTexture());
    }
	if (m_DebugTextureToDraw == 5) {
		m_DrawScreenQuadPass->Draw(m_SSAOPass->SSAOTexture());
	}
	PerformanceTimer::StopTimer("Renderer-Misc Debug Draws");

    PerformanceTimer::StartTimer("Renderer-ImGuiRenderPass");
    m_ImGuiRenderPass->Draw();
    GLERROR("Imgui draw");
	PerformanceTimer::StopTimer("Renderer-ImGuiRenderPass");

	PerformanceTimer::StartTimer("Renderer-SwapBuffer");
    glfwSwapBuffers(m_Window);
	PerformanceTimer::StopTimer("Renderer-SwapBuffer");
}

PickData Renderer::Pick(glm::vec2 screenCoord)
{
    return m_PickingPass->Pick(screenCoord);
}

void Renderer::InitializeTextures()
{
    m_ErrorTexture = CommonFunctions::LoadTexture("Textures/Core/ErrorTexture.png", false);
    m_WhiteTexture = CommonFunctions::LoadTexture("Textures/Core/White.png", false);
}


void Renderer::SortRenderJobsByDepth(RenderScene &scene)
{
    //Sort all forward jobs so transparency is good.
    scene.Jobs.TransparentObjects.sort(Renderer::DepthSort);
    scene.Jobs.SpriteJob.sort(Renderer::DepthSort);
    scene.Jobs.Text.sort(Renderer::DepthSort);
}

void Renderer::GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type)
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

void Renderer::InitializeRenderPasses()
{
    m_PickingPass = new PickingPass(this, m_EventBroker);
    m_LightCullingPass = new LightCullingPass(this);
    m_CubeMapPass = new CubeMapPass(this);
	m_SSAOPass = new SSAOPass(this, m_Config);
    m_DrawFinalPass = new DrawFinalPass(this, m_LightCullingPass, m_CubeMapPass, m_SSAOPass);
    m_DrawScreenQuadPass = new DrawScreenQuadPass(this);
    m_DrawBloomPass = new DrawBloomPass(this, m_Config);
    m_DrawColorCorrectionPass = new DrawColorCorrectionPass(this);
  
}