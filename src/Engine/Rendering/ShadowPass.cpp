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
	// m_shadCams
}

// Compute the 8 corner points of the current view frustum
std::array<glm::vec3, 8> ShadowPass::UpdateFrustumPoints(Camera* cam, glm::vec3 center, glm::vec3 view_dir)
{
	glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 right = glm::normalize(glm::cross(view_dir, up));

	glm::vec3 farCenter = center + view_dir * cam->FarClip();
	glm::vec3 nearCenter = center + view_dir * cam->NearClip();

	up = glm::normalize(glm::cross(right, view_dir));

	float near_height = tan(cam->FOV() / 2.f) * cam->NearClip();
	float near_width = near_height * cam->AspectRatio();
	float far_height = tan(cam->FOV() / 2.f) * cam->FarClip();
	float far_width = far_height * cam->AspectRatio();

	std::array<glm::vec3, 8> frustumPoints;
	frustumPoints[0] = nearCenter - up * near_height - right * near_width;
	frustumPoints[1] = nearCenter + up * near_height - right * near_width;
	frustumPoints[2] = nearCenter + up * near_height + right * near_width;
	frustumPoints[3] = nearCenter - up * near_height + right * near_width;

	frustumPoints[4] = farCenter - up * far_height - right * far_width;
	frustumPoints[5] = farCenter + up * far_height - right * far_width;
	frustumPoints[6] = farCenter + up * far_height + right * far_width;
	frustumPoints[7] = farCenter - up * far_height + right * far_width;

	return frustumPoints;
}

// UpdateSplitDist computes the near and far distances for every frustum slice
// in camera eye space - that is, at what distance does a slice start and end
void ShadowPass::UpdateSplitDist(std::array<ShadowCamera, MAX_SPLITS> shadow_cams, float near_distance, float far_distance)
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

glm::mat4 ShadowPass::FindNewFrustum(ShadowCamera shadow_cam)
{
	float maxX = -1000.0f;
	float maxY = -1000.0f;
	float maxZ;
	float minX = 1000.0f;
	float minY = 1000.0f;
	float minZ;

	glm::vec4 transf = glm::vec4(shadow_cam.frustumCorners[0], 1.f);

	//if (transf.x > maxX) maxX = transf.x;
	//if (transf.x < minX) minX = transf.x;
	//if (transf.y > maxY) maxY = transf.y;
	//if (transf.y < minY) minY = transf.y;

	for (int i = 0; i < 8; i++)
	{
		transf = glm::vec4(shadow_cam.frustumCorners[i], 1.f);

		transf.x /= transf.w;
		transf.y /= transf.w;

		if (transf.x > maxX) maxX = transf.x;
		if (transf.x < minX) minX = transf.x;
		if (transf.y > maxY) maxY = transf.y;
		if (transf.y < minY) minY = transf.y;
	}

	glm::mat4 p = glm::ortho(minX, maxX, minY, maxY, m_NearFarPlane[Near], m_NearFarPlane[Far]);

	return p;
}

glm::mat4 ShadowPass::ApplyCropMatrix(ShadowCamera& shadow_cam, glm::mat4 m, glm::mat4 v)
{
	glm::mat4 shad_modelview;
	glm::mat4 shad_proj;
	glm::mat4 shad_crop;
	glm::mat4 shad_mvp;
	float maxX = -1000.0f;
	float maxY = -1000.0f;
	float maxZ;
	float minX = 1000.0f;
	float minY = 1000.0f;
	float minZ;

	glm::mat4 nv_mvp;
	glm::vec4 transf;

	shad_modelview = m * v;
	nv_mvp = shad_modelview;

	transf = nv_mvp * glm::vec4(shadow_cam.frustumCorners[0], 1.f);
	minZ = transf.z;
	maxZ = transf.z;

	for (int i = 1; i < 8; i++) {
		transf = nv_mvp * glm::vec4(shadow_cam.frustumCorners[i], 1.f);
		if (transf.z > maxZ) {
			maxZ = transf.z;
		}
		if (transf.z < minZ) {
			minZ = transf.z;
		}
	}

	// make sure all relevant shadow casters are included here

	shad_proj = glm::ortho(-1.f, 1.f, -1.f, 1.f, m_NearFarPlane[0], m_NearFarPlane[1]);

	//return shad_proj;
	
	shad_mvp = shad_proj * shad_modelview;
	
	nv_mvp = shad_mvp;

	for (int i = 0; i < 8; i++)
	{
		transf = nv_mvp * glm::vec4(shadow_cam.frustumCorners[i], 1.0f);

		transf.x /= transf.w;
		transf.y /= transf.w;

		if (transf.x > maxX) maxX = transf.x;
		if (transf.x < minX) minX = transf.x;
		if (transf.y > maxY) maxY = transf.y;
		if (transf.y < minY) minY = transf.y;
	}

	float scaleX = 2.0f / (maxX - minX);
	float scaleY = 2.0f / (maxY - minY);
	float offsetX = -0.5f*(maxX + minX)*scaleX;
	float offsetY = -0.5f*(maxY + minY)*scaleY;

	nv_mvp = glm::mat4();
	nv_mvp[0][0] = scaleX;
	nv_mvp[1][1] = scaleY;
	nv_mvp[0][3] = offsetX;
	nv_mvp[1][3] = offsetY;
	glm::transpose(nv_mvp);

	shad_crop = nv_mvp;
	shad_crop *= shad_proj;

	//return nv_mvp;
	//return shad_crop;
	return glm::mat4();
}

void MakeShadowMap(glm::mat4 m, glm::mat4 v, glm::mat4 p, glm::vec3 light_dir)
{
	//float shad_modelview[16];

	glDisable(GL_TEXTURE_2D);

	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.f), light_dir, glm::vec3(-1.f, 0.f, 0.f));









	glEnable(GL_TEXTURE_2D);
}

glm::mat4 ShadowPass::CalculateFrustum(RenderScene & scene, std::shared_ptr<DirectionalLightJob> directionalLightJob, glm::mat4& p, glm::mat4& v, ShadowCamera shad_cam)
{
	p = glm::ortho(m_LRBT[Left], m_LRBT[Right], m_LRBT[Bottom], m_LRBT[Top], m_NearFarPlane[Near], m_NearFarPlane[Far]);
	v = glm::lookAt(glm::vec3(0.f) + shad_cam.camera->Position(), glm::vec3(directionalLightJob->Direction) + shad_cam.camera->Position(), glm::vec3(-1.f, 0.f, 0.f));

	return p * v;
}

void ShadowPass::InitializeFrameBuffers()
{
    // Depth texture
    glGenTextures(m_CurrentNrOfSplits, m_DepthMap.data());

	for (int i = 0; i < m_CurrentNrOfSplits; i++) {
		glBindTexture(GL_TEXTURE_2D, m_DepthMap[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolutionSizeWidth / (1 + i), resolutionSizeHeigth + (1 + i), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::vec4(1.f).data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

		m_DepthBuffer[i].AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_DepthMap[i], GL_DEPTH_ATTACHMENT)));
		m_DepthBuffer[i].Generate();
	}

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

void ShadowPass::InitializeLightCameras()
{
	//for (int i = 0; i < MAX_SPLITS; i++) {
	//	m_shadCams[i].camera = new Camera(1.f, );
	//	Camera.
	//}
}

void ShadowPass::ClearBuffer()
{
	for (int i = 0; i < m_CurrentNrOfSplits; i++) {
		m_DepthBuffer[i].Bind();
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_DepthBuffer[i].Unbind();
	}
}

void ShadowPass::Draw(RenderScene & scene)
{
	ImGui::DragFloat4("ShadowMapCam", m_LRBT, 1.f, -1000.f, 1000.f);
	ImGui::DragFloat2("ShadowMapNearFar", m_NearFarPlane, 1.f, -1000.f, 1000.f);
	ImGui::Checkbox("EnableShadow", &m_ShadowOn);
	ImGui::DragInt("ShadowLevel", &m_ShadowLevel, 0.05f, 0, m_CurrentNrOfSplits - 1);

	for (int i = 0; i < m_CurrentNrOfSplits; i++) {
		m_shadCams[i].camera = new Camera(*scene.Camera);
		//m_shadCams[i].frustumCorners = tempPoints;
	}

	UpdateSplitDist(m_shadCams, scene.Camera->NearClip(), scene.Camera->FarClip());
	
	for (int i = 0; i < m_CurrentNrOfSplits; i++) {
		m_shadCams[i].frustumCorners = UpdateFrustumPoints(m_shadCams[i].camera, m_shadCams[i].camera->Position(), m_shadCams[i].camera->Forward());

		ShadowPassState* state = new ShadowPassState(m_DepthBuffer[i].GetHandle());

		GLuint shaderHandle = m_ShadowProgram->GetHandle();
		m_ShadowProgram->Bind();

		glViewport(0, 0, resolutionSizeWidth / (1 + i), resolutionSizeHeigth);
		glDisable(GL_TEXTURE_2D);
		glCullFace(GL_FRONT);
		//state->Disable(GL_CULL_FACE);

		//m_LightProjection = FindNewFrustum(m_shadCams[0], m_LightProjection, m_LightProjection);

		if (m_ShadowOn == true)
		{
			for (auto &job : scene.DirectionalLightJobs) {
				auto directionalLightJob = std::dynamic_pointer_cast<DirectionalLightJob>(job);

				if (directionalLightJob) {
					m_LightSpaceMatrix = CalculateFrustum(scene, directionalLightJob, m_LightProjection, m_LightView, m_shadCams[i]);
					m_LightProjection = FindNewFrustum(m_shadCams[i]);

					glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_LightProjection));
					glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_LightView));

					GLERROR("ShadowLight ERROR");

					for (auto &objectJob : scene.OpaqueObjects) {
						auto modelJob = std::dynamic_pointer_cast<ModelJob>(objectJob);

						glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));

						//glm::mat4 proj_mat = ApplyCropMatrix(m_shadCams[0], modelJob->Matrix, m_LightView);
						//glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(proj_mat));

						glBindVertexArray(modelJob->Model->VAO);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
						glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));

						GLERROR("Shadow Draw ERROR");
					}
				}
				m_DepthBuffer[i].Unbind();

				delete state;
			}
		}

		glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
		glEnable(GL_TEXTURE_2D);
		glCullFace(GL_BACK);

		m_ShadowProgram->Unbind();
	}
}
