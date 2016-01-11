#include "Rendering/Renderer.h"

void Renderer::Initialize()
{
	InitializeWindow();
	// Create default camera
	m_DefaultCamera = new ::Camera((float)m_Resolution.Width / m_Resolution.Height, glm::radians(90.0f), 0.01f, 5000.f);
	m_DefaultCamera->SetPosition(glm::vec3(0, 1, 10));
	if (m_Camera == nullptr) {
		m_Camera = m_DefaultCamera;
	}
    m_DebugCameraInputController = std::make_shared<DebugCameraInputController<Renderer>>(m_EventBroker, -1);
    InitializeRenderPasses();

	glfwSwapInterval(m_VSYNC);
	InitializeShaders();
    InitializeTextures();

    m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.obj");
    m_UnitQuad = ResourceManager::Load<Model>("Models/Core/UnitQuad.obj");
    m_UnitSphere = ResourceManager::Load<Model>("Models/Core/UnitSphere.obj");

    m_ImGuiRenderPass = new ImGuiRenderPass(this, m_EventBroker);
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
	glfwSetWindowTitle(m_Window, ss.str().c_str());

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		LOG_ERROR("GLEW: Initialization failed");
		exit(EXIT_FAILURE);
	}
}

void Renderer::InitializeShaders()
{
    m_BasicForwardProgram = ResourceManager::Load<ShaderProgram>("#m_BasicForwardProgram");

    m_DrawScreenQuadProgram = ResourceManager::Load<ShaderProgram>("#DrawScreenQuadProgram");
    m_DrawScreenQuadProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/DrawScreenQuad.vert.glsl")));
    m_DrawScreenQuadProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/DrawScreenQuad.frag.glsl")));
    m_DrawScreenQuadProgram->Compile();
    m_DrawScreenQuadProgram->Link();
}

void Renderer::InputUpdate(double dt)
{
    m_DebugCameraInputController->Update(dt);
    m_Camera->SetOrientation(m_DebugCameraInputController->Orientation());
	m_Camera->SetPosition(m_DebugCameraInputController->Position());
}

void Renderer::Update(double dt)
{
    m_EventBroker->Process<Renderer>();
    InputUpdate(dt);
    m_ImGuiRenderPass->Update(dt);
}

void Renderer::Draw(RenderQueueCollection& rq)
{
    m_PickingPass->Draw(rq);
    //DrawScreenQuad(m_PickingPass->PickingTexture());
    m_LightCullingPass->FillLightList(rq);
    m_LightCullingPass->CullLights();
    
    //m_DrawScenePass->Draw(rq);
    m_DrawFinalPass->Draw(rq);
    glClearColor(255.f / 255, 163.f / 255, 176.f / 255, 1.f);
    GLERROR("Renderer::Draw m_DrawScenePass->Draw");
    m_ImGuiRenderPass->Draw();
	glfwSwapBuffers(m_Window);
}

void Renderer::DrawScreenQuad(GLuint textureToDraw)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);


    m_DrawScreenQuadProgram->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureToDraw);

    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->TextureGroups[0].EndIndex - m_ScreenQuad->TextureGroups[0].StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->TextureGroups[0].StartIndex);
}

void Renderer::InitializeTextures()
{
    m_ErrorTexture=ResourceManager::Load<Texture>("Textures/Core/ErrorTexture.png");
    m_WhiteTexture=ResourceManager::Load<Texture>("Textures/Core/Blank.png");
}

void Renderer::GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type)
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, dimensions.x, dimensions.y, 0, format, type, NULL);//TODO: Renderer: Fix the precision and Resolution
    GLERROR("Texture initialization failed");
}

void Renderer::InitializeRenderPasses()
{
    m_DrawScenePass = new DrawScenePass(this);
    m_PickingPass = new PickingPass(this, m_EventBroker);
    m_LightCullingPass = new LightCullingPass(this);
    m_DrawFinalPass = new DrawFinalPass(this, m_LightCullingPass);
}