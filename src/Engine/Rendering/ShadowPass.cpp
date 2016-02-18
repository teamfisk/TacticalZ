#include "Rendering/ShadowPass.h"

ShadowPass::ShadowPass(IRenderer * renderer)
{
    m_Renderer = renderer;

    //InitializeTextures();
    InitializeFrameBuffers();
    InitializeShaderPrograms();
}

ShadowPass::~ShadowPass()
{

}

// Compute the 8 corner points of the current view frustum
std::array<glm::vec3, 8> ShadowPass::UpdateFrustumPoints(Camera* cam, glm::vec3 center, glm::vec3 view_dir)
{
	glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 right = glm::normalize(glm::cross(view_dir, up));

	glm::vec3 farCenter = center + view_dir * cam->FarClip();
	glm::vec3 nearCenter = center + view_dir * cam->NearClip();

	up = glm::normalize(glm::cross(right, view_dir));

	float near_height = tan(cam->FOV() / 2.0f) * cam->NearClip();
	float near_width = near_height * cam->AspectRatio();
	float far_height = tan(cam->FOV() / 2.0f) * cam->FarClip();
	float far_width = far_height * cam->AspectRatio();

	std::array<glm::vec3, 8> frustumPoints;
	frustumPoints[0] = nearCenter - up*near_height - right*near_width;
	frustumPoints[1] = nearCenter + up*near_height - right*near_width;
	frustumPoints[2] = nearCenter + up*near_height + right*near_width;
	frustumPoints[3] = nearCenter - up*near_height + right*near_width;
	
	frustumPoints[4] = farCenter - up*far_height - right*far_width;
	frustumPoints[5] = farCenter + up*far_height - right*far_width;
	frustumPoints[6] = farCenter + up*far_height + right*far_width;
	frustumPoints[7] = farCenter - up*far_height + right*far_width;

	return frustumPoints;
}

// UpdateSplitDist computes the near and far distances for every frustum slice
// in camera eye space - that is, at what distance does a slice start and end
void ShadowPass::UpdateSplitDist(std::array<ShadowCamera, MAX_SPLITS> shadow_cams, float far_distance, float near_distance)
{
	float lambda = m_SplitWeight;
	float ratio = far_distance / near_distance;

	shadow_cams[0].camera->SetNearClip(near_distance);

	for (int i = 1; i < m_CurrentNrOfSplits; i++) {
		float si = i / static_cast<float>(m_CurrentNrOfSplits);

		shadow_cams[i].camera->SetNearClip(lambda * (near_distance * powf(ratio, si)) + (1 - lambda) * (near_distance + (far_distance - near_distance) * si));
		shadow_cams[i - 1].camera->SetFarClip(shadow_cams[i].camera->NearClip() * 1.005f);
	}

	shadow_cams[m_CurrentNrOfSplits - 1].camera->SetFarClip(far_distance);
}

float applyCropMatrix()
{

}

// GLM wants a point for glm::lookAt, brute out a point that is on the light direction's tangent.
glm::vec3 ShadowPass::LightDirectionToPoint(glm::vec4 direction)
{
	return -glm::normalize(glm::vec3(direction));
}

void ShadowPass::CalculateFrustum(RenderScene & scene, std::shared_ptr<DirectionalLightJob> directionalLightJob)
{
	//m_LightView = glm::lookAt(LightDirectionToPoint(directionalLightJob->Direction), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	m_LightProjection = glm::ortho(m_LRBT[Left], m_LRBT[Right], m_LRBT[Bottom], m_LRBT[Top], m_NearFarPlane[Near], m_NearFarPlane[Far]);
	
	glm::vec3 LightPoint = LightDirectionToPoint(directionalLightJob->Direction);

	float CameraDistance = scene.Camera->FarClip() - scene.Camera->NearClip();
	float CameraPercentiles[3] = { 0.f, CameraDistance * 0.2f, CameraDistance * 0.65f };

	
	glm::vec3 point = scene.Camera->Position() + (scene.Camera->Forward() * 30.f);
	
	m_LightView = glm::lookAt(LightPoint + point, point, glm::vec3(0.f, 1.f, 0.f));

	m_LightSpaceMatrix = m_LightProjection * m_LightView;

	//scene.Camera->
}

void ShadowPass::InitializeFrameBuffers()
{
    
//     glGenRenderbuffers(1, &m_DepthFBO);
//     glBindRenderbuffer(GL_RENDERBUFFER, m_DepthFBO);
//     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);

    // Depth texture
    glGenTextures(1, &m_DepthMap);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolutionSizeWidth, resolutionSizeHeigth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::vec4(1.f).data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);


    m_DepthBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_DepthMap, GL_DEPTH_ATTACHMENT)));
    //m_DepthBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_DepthMap, GL_COLOR_ATTACHMENT0)));
    m_DepthBuffer.Generate();

    GLERROR("depthMap failed");

}

void ShadowPass::InitializeShaderPrograms()
{
    m_ShadowProgram = ResourceManager::Load<ShaderProgram>("#ShadowProgram");
    m_ShadowProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Shadow.vert.glsl")));
    m_ShadowProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Shadow.frag.glsl")));
    m_ShadowProgram->Compile();
    m_ShadowProgram->BindFragDataLocation(0, "ShadowMap");
    m_ShadowProgram->Link();
}

void ShadowPass::ClearBuffer()
{
    m_DepthBuffer.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_DepthBuffer.Unbind();
}

void ShadowPass::Draw(RenderScene & scene)
{
    ShadowPassState* state = new ShadowPassState(m_DepthBuffer.GetHandle());

    GLuint shaderHandle = m_ShadowProgram->GetHandle();
    m_ShadowProgram->Bind();

    glViewport(0, 0, resolutionSizeWidth, resolutionSizeHeigth);

	glCullFace(GL_FRONT);
    //state->Disable(GL_CULL_FACE);

    ImGui::DragFloat4("ShadowMapCam", m_LRBT, 1.f, -1000.f, 1000.f);
    ImGui::DragFloat2("ShadowMapNearFar", m_NearFarPlane, 1.f, -1000.f, 1000.f);
    ImGui::Checkbox("EnableShadow", &m_ShadowOn);

    if (m_ShadowOn == true)
    {
        for (auto &job : scene.DirectionalLightJobs) {
            auto directionalLightJob = std::dynamic_pointer_cast<DirectionalLightJob>(job);
        
            if(directionalLightJob) {
				CalculateFrustum(scene, directionalLightJob);

                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_LightProjection));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_LightView));

                GLERROR("ShadowLight ERROR");

                for (auto &objectJob : scene.OpaqueObjects) {
                    auto modelJob = std::dynamic_pointer_cast<ModelJob>(objectJob);
                
                    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));

                    glBindVertexArray(modelJob->Model->VAO);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
                    glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));

                    GLERROR("Shadow Draw ERROR");

                }
          
            }

            m_DepthBuffer.Unbind();

            delete state;


        }
    }



    glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    glCullFace(GL_BACK);
    m_ShadowProgram->Unbind();





}
