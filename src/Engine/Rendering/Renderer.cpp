#include "Rendering/Renderer.h"

void Renderer::Initialize()
{
	InitializeWindow();
	// Create default camera
	m_DefaultCamera = new ::Camera((float)m_Resolution.Width / m_Resolution.Height, glm::radians(45.f), 0.01f, 5000.f);
	m_DefaultCamera->SetPosition(glm::vec3(0, 0, 10));
	if (m_Camera == nullptr) {
		m_Camera = m_DefaultCamera;
	}
    TEMPCreateLights();
    InitializeRenderPasses();

	glfwSwapInterval(m_VSYNC);
	InitializeShaders();
    InitializeTextures();
    InitializeFrameBuffers();
    InitializeSSBOs();
    //CalculateFrustum();

    m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.obj");
    m_UnitQuad = ResourceManager::Load<Model>("Models/Core/UnitQuad.obj");
    m_UnitSphere = ResourceManager::Load<Model>("Models/Core/UnitSphere.obj");
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
	m_BasicForwardProgram.AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/BasicForward.vert.glsl")));
	m_BasicForwardProgram.AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/BasicForward.frag.glsl")));
	m_BasicForwardProgram.Compile();
	m_BasicForwardProgram.Link();

    m_DrawScreenQuadProgram.AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/DrawScreenQuad.vert.glsl")));
    m_DrawScreenQuadProgram.AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/DrawScreenQuad.frag.glsl")));
    m_DrawScreenQuadProgram.Compile();
    m_DrawScreenQuadProgram.Link();

    //m_CalculateFrustumProgram.AddShader(std::shared_ptr<Shader>(new ComputeShader("Shaders/GridFrustum.comp.glsl")));
    //m_CalculateFrustumProgram.Compile();
    //m_CalculateFrustumProgram.Link();

    //m_LightCullProgram.AddShader(std::shared_ptr<Shader>(new ComputeShader("Shaders/cullLights.comp.glsl")));
    //m_LightCullProgram.Compile();
    //m_LightCullProgram.Link();
}

void Renderer::InputUpdate(double dt)
{
	glm::vec3 m_Position = m_Camera->Position();
	if (glfwGetKey(m_Window, GLFW_KEY_O) == GLFW_PRESS)
	{
		m_Position = glm::vec3(0.f, 0.f, 5.f);
	}
	if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		m_Position += m_Camera->Forward() * m_CameraMoveSpeed * (float)dt;
	}
	if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		m_Position -= m_Camera->Forward() * m_CameraMoveSpeed * (float)dt;
	}
	if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		m_Position += m_Camera->Right() * m_CameraMoveSpeed * (float)dt;
	}
	if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		m_Position -= m_Camera->Right() * m_CameraMoveSpeed * (float)dt;
	}
	if (glfwGetKey(m_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		m_CameraMoveSpeed = 5.f;
	}
	else {
		m_CameraMoveSpeed = 0.5f;
	}

    static double mousePosX, mousePosY;
    glfwGetCursorPos(m_Window, &mousePosX, &mousePosY);

	if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS) {

		
		double deltaX, deltaY;
		deltaX = mousePosX - (float)Resolution().Width / 2;
		deltaY = mousePosY - (float)Resolution().Height / 2;

		float rotationY = -deltaY / 300.f;
		float rotationX = -deltaX / 300.f;
		glm::quat orientation = m_Camera->Orientation();
		
		
		orientation = orientation * glm::angleAxis<float>(rotationY, glm::vec3(1, 0, 0));
		orientation = glm::angleAxis<float>(rotationX, glm::vec3(0, 1, 0)) * orientation;
		
		m_Camera->SetOrientation(orientation);

		glfwSetCursorPos(m_Window, Resolution().Width / 2, Resolution().Height / 2);
	}
	m_Camera->SetPosition(m_Position);
}

void Renderer::Update(double dt)
{
    m_EventBroker->Process<Renderer>();
    InputUpdate(dt);
    
}

void Renderer::Draw(RenderQueueCollection& rq)
{
    //TODO: Renderer: Kanske borde vara längst upp i update.
    m_PickingPass->Draw(rq);
    //DrawScreenQuad(m_PickingPass->PickingTexture());
    //CullLights();
    
    //DrawScene(rq);
    m_DrawScenePass->Draw(rq);
	glfwSwapBuffers(m_Window);
}

void Renderer::DrawScene(RenderQueueCollection& rq)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //TODO: Render: Clean up draw code
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(255.f / 255, 163.f / 255, 176.f / 255, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //TODO: Render: Add code for more jobs than modeljobs.
    for (auto &job : rq.Forward) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
        if (modelJob) {
            GLuint ShaderHandle = m_BasicForwardProgram.GetHandle();

            m_BasicForwardProgram.Bind();
            //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->ModelMatrix));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_Camera->ProjectionMatrix()));
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
    GLERROR("DrawScene Error");
}

void Renderer::DrawScreenQuad(GLuint textureToDraw)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);


    m_DrawScreenQuadProgram.Bind();
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

void Renderer::InitializeFrameBuffers()//TODO: Renderer: Get this to a better location, as its really big
{
    //glGenRenderbuffers(1, &m_DepthBuffer);
    //glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBuffer);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Resolution.Width, m_Resolution.Height);
    //
    //m_PickingBuffer.AddResource(std::shared_ptr<BufferResource>(new RenderBuffer(&m_DepthBuffer, GL_DEPTH_ATTACHMENT)));
    //m_PickingBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_PickingTexture, GL_COLOR_ATTACHMENT0)));
    //m_PickingBuffer.Generate();
}

void Renderer::InitializeSSBOs()
{
    printf("Size: %i\n", sizeof(m_Frustums));
    glGenBuffers(1, &m_FrustumSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_FrustumSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_Frustums), &m_Frustums, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_FrustumSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLERROR("m_FrustumSSBO");

    glGenBuffers(1, &m_LightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_PointLights), &m_PointLights, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLERROR("m_LightSSBO");


    glGenBuffers(1, &m_LightGridSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightGridSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_LightGrid), &m_LightGrid, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightGridSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLERROR("m_LightGridSSBO");


    glGenBuffers(1, &m_LightOffsetSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightOffsetSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_LightOffset), &m_LightOffset, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_LightOffsetSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLERROR("m_LightOffsetSSBO");


    glGenBuffers(1, &m_LightIndexSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightIndexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_LightIndex), &m_LightIndex, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightIndexSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLERROR("m_LightIndexSSBO");

}

void Renderer::InitializeRenderPasses()
{
    m_PickingPass = new PickingPass(this, m_EventBroker);
    m_DrawScenePass = new DrawScenePass(this);
}

void Renderer::CalculateFrustum()
{
    GLERROR("CalculateFrustum Error-1");
    m_CalculateFrustumProgram.Bind();
    
    GLERROR("CalculateFrustum Error1");
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_FrustumSSBO);
    GLERROR("CalculateFrustum Error2");
    glUniformMatrix4fv(glGetUniformLocation(m_CalculateFrustumProgram.GetHandle(), "P"), 1, false, glm::value_ptr(m_Camera->ProjectionMatrix()));
    GLERROR("CalculateFrustum Error3");
    glUniform2f(glGetUniformLocation(m_CalculateFrustumProgram.GetHandle(), "ScreenDimensions"), m_Resolution.Width, m_Resolution.Height);
    GLERROR("CalculateFrustum Error4");
    glDispatchCompute(5, 3, 1);
    GLERROR("CalculateFrustum Error5");

}

void Renderer::TEMPCreateLights()
{
    for (int i = 0; i < NUM_LIGHTS; i++) {
        m_PointLights[i].Position = glm::vec4(i, 0.f, 0.f, 0.f);
        m_PointLights[i].Color = glm::vec4(1.f, 0.5f, 0.f + i*0.1f, 1.f);
    }
}

void Renderer::CullLights()
{
    m_LightCullProgram.Bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_FrustumSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightGridSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_LightOffsetSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightIndexSSBO);
    glDispatchCompute(m_Resolution.Width / TILE_SIZE, m_Resolution.Height / TILE_SIZE, 1);
    GLERROR("CullLights Error");

}

