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

	glfwSwapInterval(m_VSYNC);
	m_ErrorTexture = ResourceManager::Load<Texture>("Textures/Core/ErrorTexture.png");
	m_WhiteTexture = ResourceManager::Load<Texture>("Textures/Core/Blank.png");
	InitializeShaders();
	ModelsToDraw();
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
}

void Renderer::ModelsToDraw()
{
	Model *m = ResourceManager::Load<Model>("Models/ScaleWidget.obj");
	EnqueueModel(m);
	Model *m2 = ResourceManager::Load<Model>("Models/TranslationWidget.obj");
	EnqueueModel(m2);
	Model *m3 = ResourceManager::Load<Model>("Models/RotationWidget.obj");
	EnqueueModel(m3);
}

void Renderer::EnqueueModel(Model* model)
{
	for (auto texGroup : model->TextureGroups)
	{
		ModelJob job;
		job.TextureID = (texGroup.Texture) ? texGroup.Texture->ResourceID : 0;
		job.DiffuseTexture = texGroup.Texture.get();
		job.NormalTexture = texGroup.NormalMap.get();
		job.SpecularTexture = texGroup.SpecularMap.get();
		job.Model = model;
		job.StartIndex = texGroup.StartIndex;
		job.EndIndex = texGroup.EndIndex;
		job.ModelMatrix =  model->m_Matrix;
		//job.ModelMatrix = modelMatrix * model->m_Matrix; TODO: Render: Make sure modelMatrix work
		//job.Color = modelComponent->Color; TODO: Render: Take color from kd in .mtl or a value from modelcomponent.
		job.Color = glm::vec4(1.f);

		//TODO: Render: Skeleton animations
		//if (model->m_Skeleton != nullptr && animationComponent != nullptr) {
		//	job.Skeleton = model->m_Skeleton;
		//	job.AnimationName = animationComponent->Name;
		//	job.AnimationTime = animationComponent->Time;
		//	job.NoRootMotion = animationComponent->NoRootMotion;
		//}

		m_TempRQ.Forward.Add(job);
	}
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

	if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		static double mousePosX, mousePosY;

		glfwGetCursorPos(m_Window, &mousePosX, &mousePosY);
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
	InputUpdate(dt);
}

void Renderer::Draw(RenderQueueCollection& rq)
{
	//TODO: Render: Clean up draw code
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glClearColor(255.f / 255, 163.f / 255, 176.f / 255, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//TODO: Render: Add code for more jobs than modeljobs.
	for (auto &job : m_TempRQ.Forward) {
		auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
		if (modelJob) {
			GLuint ShaderHandle = m_BasicForwardProgram.GetHandle();

			m_BasicForwardProgram.Bind();
			//TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
			glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
			glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_Camera->ViewMatrix()));
			glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_Camera->ProjectionMatrix()));

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
	glfwSwapBuffers(m_Window);
}

