#include "Rendering/ShadowPass.h"

ShadowPass::ShadowPass(IRenderer * renderer, int shadow_res_x, int shadow_res_y)
{
	m_Renderer = renderer;
	m_ResolutionSizeWidth = shadow_res_x;
	m_ResolutionSizeHeight = shadow_res_y;

	CheckStatus(true);
	InitializeShaderPrograms();
}

ShadowPass::ShadowPass(IRenderer * renderer)
{
	m_Renderer = renderer;

	CheckStatus(true);
	InitializeShaderPrograms();
}

ShadowPass::~ShadowPass()
{
	CommonFunctions::DeleteTexture(&m_DepthMap);
}

void ShadowPass::DebugGUI()
{
	ImGui::DragFloat2("ShadowMapNearFar", m_NearFarPlane, 1.f, -1000.f, 1000.f);
	ImGui::DragFloat("ShadowClippingWeight", &m_SplitWeight, 0.001f, 0.f, 1.f);
	ImGui::Checkbox("ShadowTransparentObjects", &m_TransparentObjects);
	ImGui::Checkbox("ShadowOnTextureAlphas", &m_TexturedShadows);
}

void ShadowPass::InitializeCameras(RenderScene & scene)
{
	for (int i = 0; i < m_CurrentNrOfSplits; i++) {
		m_shadowFrusta[i].AspectRatio = scene.Camera->AspectRatio();
		m_shadowFrusta[i].FOV = scene.Camera->FOV();
	}
}

// UpdateSplitDist computes the near and far distances for every frustum slice
// in camera eye space - that is, at what distance does a slice start and end
void ShadowPass::UpdateSplitDist(std::array<ShadowFrustum, MAX_SPLITS>& frusta, float near_distance, float far_distance)
{
	float lambda = m_SplitWeight;
	float ratio = far_distance / near_distance;

	frusta[0].NearClip = near_distance;

	for (int i = 1; i < m_CurrentNrOfSplits; i++) {
		float si = i / static_cast<float>(m_CurrentNrOfSplits);

		frusta[i].NearClip = lambda * (near_distance * powf(ratio, si)) + (1 - lambda) * (near_distance + (far_distance - near_distance) * si);
		frusta[i - 1].FarClip = frusta[i].NearClip * 1.005f;
	}

	frusta[m_CurrentNrOfSplits - 1].FarClip = far_distance;
}

void ShadowPass::UpdateFrustumPoints(ShadowFrustum& frustum, glm::mat4 p, glm::mat4 v)
{
	std::array<glm::vec4, 8> CornerPoint = {
		glm::vec4(-1.f, -1.f,	-1.f,	1.f),
		glm::vec4(-1.f, 1.f,	-1.f,	1.f),
		glm::vec4(1.f,	1.f,	-1.f,	1.f),
		glm::vec4(1.f,	-1.f,	-1.f,	1.f),
		glm::vec4(-1.f,	-1.f,	1.f,	1.f),
		glm::vec4(-1.f,	1.f,	1.f,	1.f),
		glm::vec4(1.f,	1.f,	1.f,	1.f),
		glm::vec4(1.f,	-1.f,	1.f,	1.f)
	};

	for (int i = 0; i < 8; i++) {
		glm::vec4 NDC = glm::inverse(p) * CornerPoint[i];
		NDC = NDC / NDC.w;
		frustum.CornerPoint[i] = glm::vec3(glm::inverse(v) * NDC);
	}
}

// Compute the 8 corner points of the current view frustum in world space
void ShadowPass::UpdateFrustumPoints(ShadowFrustum& frustum, glm::vec3 camera_position, glm::vec3 view_dir)
{
	glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 right = glm::normalize(glm::cross(view_dir, up));

	glm::vec3 far_center = camera_position + glm::normalize(view_dir) * frustum.FarClip;
	glm::vec3 near_center = camera_position + glm::normalize(view_dir) * frustum.NearClip;
	frustum.MiddlePoint = near_center + (far_center - near_center) * 0.5f;

	up = glm::normalize(glm::cross(right, view_dir));

	// these heights and widths are half the heights and widths of the near and far plane rectangles.
	float near_height = tan(frustum.FOV / 2.f) * frustum.NearClip;
	float near_width = near_height * frustum.AspectRatio;
	float far_height = tan(frustum.FOV / 2.f) * frustum.FarClip;
	float far_width = far_height * frustum.AspectRatio;

	frustum.CornerPoint[0] = near_center - up * near_height - right * near_width;
	frustum.CornerPoint[1] = near_center + up * near_height - right * near_width;
	frustum.CornerPoint[2] = near_center + up * near_height + right * near_width;
	frustum.CornerPoint[3] = near_center - up * near_height + right * near_width;

	frustum.CornerPoint[4] = far_center - up * far_height - right * far_width;
	frustum.CornerPoint[5] = far_center + up * far_height - right * far_width;
	frustum.CornerPoint[6] = far_center + up * far_height + right * far_width;
	frustum.CornerPoint[7] = far_center - up * far_height + right * far_width;
}

float ShadowPass::FindRadius(ShadowFrustum& frustum)
{
	float radius = 0.f;

	for (int i = 0; i < 8; i++) {
		float distance = glm::distance(frustum.MiddlePoint, frustum.CornerPoint[i]);
		if (distance > radius) {
			radius = distance;
		}
	}

	frustum.Radius = radius;
	return radius;
}

void ShadowPass::InitializeFrameBuffers()
{
	// Depth texture
	glDeleteTextures(1, &m_DepthMap);
	glGenTextures(1, &m_DepthMap);

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_DepthMap);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT16, m_ResolutionSizeWidth, m_ResolutionSizeHeight, m_CurrentNrOfSplits);

	//glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, m_ResolutionSizeWidth, m_ResolutionSizeHeight, m_CurrentNrOfSplits, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, glm::vec4(1.f).data);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	m_DepthBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2DArray(&m_DepthMap, GL_DEPTH_ATTACHMENT)));
	m_DepthBuffer.Generate();

	GLERROR("depthMap failed END");
}

void ShadowPass::InitializeShaderPrograms()
{
	m_ShadowProgram = ResourceManager::Load<ShaderProgram>("#ShadowProgram");
	m_ShadowProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Shadow.vert.glsl")));
	m_ShadowProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Shadow.frag.glsl")));
	m_ShadowProgram->Compile();
	m_ShadowProgram->BindFragDataLocation(0, "ShadowMap");
	m_ShadowProgram->Link();

	m_ShadowProgramSkinned = ResourceManager::Load<ShaderProgram>("#ShadowProgramSkinned");
	m_ShadowProgramSkinned->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/ShadowSkinned.vert.glsl")));
	m_ShadowProgramSkinned->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Shadow.frag.glsl")));
	m_ShadowProgramSkinned->Compile();
	m_ShadowProgramSkinned->BindFragDataLocation(0, "ShadowMap");
	m_ShadowProgramSkinned->Link();
}

void ShadowPass::ClearBuffer()
{
	m_DepthBuffer.Bind();

	for (int i = 0; i < m_CurrentNrOfSplits; i++) {
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthMap, 0, i);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	m_DepthBuffer.Unbind();
}

void ShadowPass::PointsToLightspace(ShadowFrustum& frustum, glm::mat4 v)
{
	float left = INFINITY;
	float right = -INFINITY;
	float bottom = INFINITY;
	float top = -INFINITY;

	for (int i = 0; i < 8; i++)
	{
		glm::vec3 tempPoint = glm::vec3(v * glm::vec4(frustum.CornerPoint[i], 1.f));

		if (tempPoint.x < left) { left = tempPoint.x; }
		if (tempPoint.x > right) { right = tempPoint.x; }
		if (tempPoint.y < bottom) { bottom = tempPoint.y; }
		if (tempPoint.y > top) { top = tempPoint.y; }
	}

	frustum.LRBT = { left, right, bottom, top };
}

void ShadowPass::RadiusToLightspace(ShadowFrustum& frustum)
{
	float quantizationStep = 1.0f / m_ResolutionSizeHeight;

	float left = -frustum.Radius;
	float right = frustum.Radius;
	float bottom = -frustum.Radius;
	float top = frustum.Radius;

	frustum.LRBT = { left, right, bottom, top };
}

void ShadowPass::CheckStatus(bool shadowStatus)
{
	if (m_EnableShadows == shadowStatus) {
		return;
	}

	m_EnableShadows = shadowStatus;

	if (m_EnableShadows == false) {
		CommonFunctions::DeleteTexture(&m_DepthMap);
		return;
	}

	InitializeFrameBuffers();
}

void ShadowPass::Draw(RenderScene & scene)
{
	if (!m_EnableShadows) {
		return;
	}

	InitializeCameras(scene);
	UpdateSplitDist(m_shadowFrusta, scene.Camera->NearClip(), scene.Camera->FarClip());

	ShadowPassState* state = new ShadowPassState(m_DepthBuffer.GetHandle());

	glViewport(0, 0, m_ResolutionSizeWidth, m_ResolutionSizeHeight);

	for (int i = 0; i < m_CurrentNrOfSplits; i++) {
		UpdateFrustumPoints(m_shadowFrusta[i], scene.Camera->Position(), scene.Camera->Forward());

		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthMap, 0, i);

		GLuint shaderHandle;

		for (auto &job : scene.Jobs.DirectionalLight) {

			auto directionalLightJob = std::dynamic_pointer_cast<DirectionalLightJob>(job);

			if (directionalLightJob) {
				m_LightView[i] = glm::lookAt(glm::vec3(-directionalLightJob->Direction) + m_shadowFrusta[i].MiddlePoint, m_shadowFrusta[i].MiddlePoint, glm::vec3(0.f, 1.f, 0.f));

				PointsToLightspace(m_shadowFrusta[i], m_LightView[i]);
				//FindRadius(m_shadowFrusta[i]);
				//RadiusToLightspace(m_shadowFrusta[i]);
				m_LightProjection[i] = glm::ortho(m_shadowFrusta[i].LRBT[LEFT], m_shadowFrusta[i].LRBT[RIGHT], m_shadowFrusta[i].LRBT[BOTTOM], m_shadowFrusta[i].LRBT[TOP], m_NearFarPlane[NEAR], m_NearFarPlane[FAR]);


				m_ShadowProgram->Bind();
				shaderHandle = m_ShadowProgram->GetHandle();
				glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_LightProjection[i]));
				glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_LightView[i]));

				m_ShadowProgramSkinned->Bind();
				shaderHandle = m_ShadowProgramSkinned->GetHandle();
				glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_LightProjection[i]));
				glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_LightView[i]));



				GLERROR("ShadowLight ERROR");

				for (auto &objectJob : scene.Jobs.OpaqueObjects) {
					if (!std::dynamic_pointer_cast<ExplosionEffectJob>(objectJob)) {
						auto modelJob = std::dynamic_pointer_cast<ModelJob>(objectJob);

						if (!modelJob->Shadow) {
							continue;
						}

						if (modelJob->Model->IsSkinned()) {
							m_ShadowProgramSkinned->Bind();
							shaderHandle = m_ShadowProgramSkinned->GetHandle();

							std::vector<glm::mat4> frameBones;
							if (modelJob->BlendTree != nullptr) {
								frameBones = modelJob->BlendTree->GetFinalPose();
							}
							else {
								frameBones = modelJob->Skeleton->GetTPose();
							}
							glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

						}
						else {
							m_ShadowProgram->Bind();
							shaderHandle = m_ShadowProgram->GetHandle();
						}

						glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
						glUniform1f(glGetUniformLocation(shaderHandle, "Alpha"), 1.f);

						glBindVertexArray(modelJob->Model->VAO);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
						glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));

						GLERROR("Shadow Draw ERROR");
					}
				}
				if (m_TransparentObjects) {
					state->CullFace(GL_BACK);
					for (auto &objectJob : scene.Jobs.TransparentObjects) {
						if (!std::dynamic_pointer_cast<ExplosionEffectJob>(objectJob)) {
							auto modelJob = std::dynamic_pointer_cast<ModelJob>(objectJob);

							if (!modelJob->Shadow) {
								continue;
							}

							if (modelJob->Model->IsSkinned()) {
								m_ShadowProgramSkinned->Bind();
								shaderHandle = m_ShadowProgramSkinned->GetHandle();

								std::vector<glm::mat4> frameBones;
								if (modelJob->BlendTree != nullptr) {
									frameBones = modelJob->BlendTree->GetFinalPose();
								}
								else {
									frameBones = modelJob->Skeleton->GetTPose();
								}
								glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

							}
							else {
								m_ShadowProgram->Bind();
								shaderHandle = m_ShadowProgram->GetHandle();
							}

							glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
							glUniform1f(glGetUniformLocation(shaderHandle, "Alpha"), modelJob->Color.a);

							if (m_TexturedShadows) {
								switch (modelJob->Type) {
								case RawModel::MaterialType::SingleTextures:
								case RawModel::MaterialType::Basic:
								{
									glActiveTexture(GL_TEXTURE24);
									if (modelJob->DiffuseTexture.size() > 0 && modelJob->DiffuseTexture[0]->Texture != nullptr) {
										glBindTexture(GL_TEXTURE_2D, modelJob->DiffuseTexture[0]->Texture->m_Texture);
										glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(modelJob->DiffuseTexture[0]->UVRepeat));
									}
									else {
										glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
										glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
									}
									break;
								}
								case RawModel::MaterialType::SplatMapping:
								{
									glActiveTexture(GL_TEXTURE24);
									glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
									glUniform2fv(glGetUniformLocation(shaderHandle, "DiffuseUVRepeat"), 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));
									break;
								}
								}
							}

							glBindVertexArray(modelJob->Model->VAO);
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
							glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));

							GLERROR("Shadow Draw ERROR");
						}
					}
					state->CullFace(GL_FRONT);
				}
			}
		}
	}

	glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
	m_DepthBuffer.Unbind();
	delete state;
}