#ifndef ShadowPass_h_
#define ShadowPass_h_

#include "IRenderer.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "../Core/EventBroker.h"
#include "../Core/World.h"
#include "ShadowPassState.h"
#include "imgui/imgui.h"

#define MAX_SPLITS 4

enum NearFar { NEAR = 0, FAR = 1 };
enum LRBT { LEFT = 0, RIGHT = 1, BOTTOM = 2, TOP = 3 };

struct Frustum
{
	float NearClip;
	float FarClip;
	float FOV;
	float AspectRatio;
	glm::vec3 MiddlePoint;
	float Radius;
	std::array<float, 4> LRBT;
	std::array<glm::vec3, 8> CornerPoint;
};

class ShadowPass
{
public:
	ShadowPass(IRenderer* renderer);
	ShadowPass(IRenderer * renderer, int ShadowResX, int ShadowResY);
	~ShadowPass();

	void InitializeFrameBuffers();
	void InitializeShaderPrograms();
	void ClearBuffer();
	void Draw(RenderScene& scene);

	GLuint DepthMap() const { return m_DepthMap; }
	std::array<glm::mat4, MAX_SPLITS> LightP() const { return m_LightProjection; }
	std::array<glm::mat4, MAX_SPLITS> LightV() const { return m_LightView; }
	std::array<float, MAX_SPLITS> FarDistance() const { return{ m_shadowFrusta[0].FarClip, m_shadowFrusta[1].FarClip, m_shadowFrusta[2].FarClip, m_shadowFrusta[3].FarClip }; }
	int CurrentNrOfSplits() const { return m_CurrentNrOfSplits; }

	void SetSplitWeight(float split_weight) { m_SplitWeight = split_weight; };
private:
	void InitializeCameras(RenderScene & scene);
	void UpdateSplitDist(std::array<Frustum, MAX_SPLITS>& frusta, float near_distance, float far_distance);
	void UpdateFrustumPoints(Frustum& frustum, glm::vec3 camera_position, glm::vec3 view_dir);
	void UpdateFrustumPoints(Frustum& frustum, glm::mat4 p, glm::mat4 v);

	void PointsToLightspace(Frustum& frustum, glm::mat4 v);

	float FindRadius(Frustum& frustum);
	void RadiusToLightspace(Frustum& frustum);

	EventBroker* m_EventBroker;
	const IRenderer* m_Renderer;

	GLuint m_DepthMap;
	FrameBuffer m_DepthBuffer;
	ShaderProgram* m_ShadowProgram;
	//ShaderProgram* m_TransparentShadowProgram;

	std::array<glm::mat4, MAX_SPLITS> m_LightProjection;
	std::array<glm::mat4, MAX_SPLITS> m_LightView;

	GLfloat m_NearFarPlane[2] = { -34.f, 27.f };
	GLuint m_ResolutionSizeWidth = 1024 * 2;
	GLuint m_ResolutionSizeHeight = 1024 * 2;

	int m_CurrentNrOfSplits = 4;
	float m_SplitWeight = 0.91f;

	std::array<Frustum, MAX_SPLITS> m_shadowFrusta;

	Texture* m_WhiteTexture = ResourceManager::Load<Texture>("Textures/Core/White.png");
};

#endif