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
	InitializeShaders();
    InitializeTextures();
    InitializeFrameBuffers();
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

    m_PickingProgram.AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Picking.vert.glsl")));
    m_PickingProgram.AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Picking.frag.glsl")));
    m_PickingProgram.Compile();
    m_PickingProgram.BindFragDataLocation(0, "TextureFragment");
    m_PickingProgram.Link();

    m_DrawScreenQuadProgram.AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/DrawScreenQuad.vert.glsl")));
    m_DrawScreenQuadProgram.AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/DrawScreenQuad.frag.glsl")));
    m_DrawScreenQuadProgram.Compile();
    m_DrawScreenQuadProgram.Link();

}

void Renderer::ModelsToDraw()
{
    
	m = ResourceManager::Load<Model>("Models/ScaleWidget.obj");
	EnqueueModel(m);
	m2 = ResourceManager::Load<Model>("Models/TranslationWidget.obj");
	EnqueueModel(m2);
	m3 = ResourceManager::Load<Model>("Models/RotationWidget.obj");
	EnqueueModel(m3);
    
	m_UnitSphere = ResourceManager::Load<Model>("Models/Core/UnitSphere.obj");
	EnqueueModel(m_UnitSphere);
    m_UnitQuad = ResourceManager::Load<Model>("Models/Core/UnitQuad.obj");
    m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.obj");

    MapModel = ResourceManager::Load<Model>("Models/DummyScene.obj");
    EnqueueModel(MapModel);
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


    static double mousePosX, mousePosY;
    glfwGetCursorPos(m_Window, &mousePosX, &mousePosY);
    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        glm::vec3 data = ScreenCoords::ToPixelData(mousePosX, m_Resolution.Height - mousePosY, m_PickingBuffer, m_DepthBuffer);
        glm::vec2 color = glm::vec2(data);
        float depth = data.z;
        
        glm::vec3 viewPos = ScreenCoords::ToWorldPos(mousePosX, m_Resolution.Height - mousePosY, depth, m_Resolution, m_Camera->ProjectionMatrix(), m_Camera->ViewMatrix());
      //  glm::vec3 worldPos = glm::vec3(glm::inverse(m_Camera->ViewMatrix()) * glm::vec4(viewPos, 1.f));

        //printf("R: %f, G: %f, Depth: %f\n", color.r, color.g, depth);
        //printf("view: x: %f, y: %f z: %f, Length: %f\n\n", viewPos.x, viewPos.y, viewPos.z, glm::length(viewPos));

        if (color != glm::vec2(0, 0)) {
            const Model* pickModel = m_PickingColorsToModels[color];

        }
    }

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
	InputUpdate(dt);
}

void Renderer::Draw(RenderQueueCollection& rq)
{
    //TODO: Renderer: Kanske borde vara längst upp i update.
    PickingPass();
    DrawScreenQuad(m_PickingTexture);
    //DrawScreenQuad(m_DepthBuffer);
    //DrawScene(rq);
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
}

void Renderer::PickingPass()
{
    m_PickingBuffer.Bind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int r = 30;
    int g = 0;
    //TODO: Render: Add code for more jobs than modeljobs.


    GLuint ShaderHandle = m_PickingProgram.GetHandle();
    m_PickingProgram.Bind();

    for (auto &job : m_TempRQ.Forward) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);

        if (modelJob) {
            glm::vec2 pickColor = glm::vec2(r/255.f, g/255.f);
            m_PickingColorsToModels[pickColor] = modelJob->Model;

            //Render picking stuff
            //TODO: Kolla upp "header/include/common" shader saken så man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_Camera->ProjectionMatrix()));
            glUniform2fv(glGetUniformLocation(ShaderHandle, "PickingColor"), 1, glm::value_ptr(pickColor));

            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElementsBaseVertex(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, 0, modelJob->StartIndex);
            r+=50;
            if(r > 255) {
                r = 0;
                g+=50;
            }
        }
    }

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
    /*
    glGenTextures(1, &m_PickingTexture);
    glBindTexture(GL_TEXTURE_2D, m_PickingTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, m_Resolution.Width, m_Resolution.Height, 0, GL_RG, GL_FLOAT, NULL);//TODO: Renderer: Fix the precision and Resolution
    GLERROR("m_PickingTexture initialization failed");
    */

    GenerateTexture(&m_PickingTexture, GL_CLAMP_TO_BORDER, GL_LINEAR,
        glm::vec2(m_Resolution.Width, m_Resolution.Height), GL_RG8, GL_RG, GL_FLOAT);
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
    glGenRenderbuffers(1, &m_DepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Resolution.Width, m_Resolution.Height);
    
    m_PickingBuffer.AddResource(std::shared_ptr<BufferResource>(new RenderBuffer(&m_DepthBuffer, GL_DEPTH_ATTACHMENT)));
    m_PickingBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_PickingTexture, GL_COLOR_ATTACHMENT0)));
    m_PickingBuffer.Generate();
}