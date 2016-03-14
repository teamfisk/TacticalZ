#ifndef ShadowPass_h__
#define ShadowPass_h__

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

struct ShadowFrustum
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

	void DebugGUI();

	GLuint DepthMap() const { return m_DepthMap; }
	std::array<glm::mat4, MAX_SPLITS> LightP() const { return m_LightProjection; }
	std::array<glm::mat4, MAX_SPLITS> LightV() const { return m_LightView; }
	std::array<float, MAX_SPLITS> FarDistance() const { std::array<float, MAX_SPLITS> f; for (int i = 0; i < MAX_SPLITS; i++) f[i] = m_shadowFrusta[i].FarClip; return f; }
	int CurrentNrOfSplits() const { return m_CurrentNrOfSplits; }

	void SetSplitWeight(float split_weight) { m_SplitWeight = split_weight; };
private:
	void InitializeCameras(RenderScene & scene);
	void UpdateSplitDist(std::array<ShadowFrustum, MAX_SPLITS>& frusta, float near_distance, float far_distance);
	void UpdateFrustumPoints(ShadowFrustum& frustum, glm::vec3 camera_position, glm::vec3 view_dir);
	void UpdateFrustumPoints(ShadowFrustum& frustum, glm::mat4 p, glm::mat4 v);

	void PointsToLightspace(ShadowFrustum& frustum, glm::mat4 v);

	float FindRadius(ShadowFrustum& frustum);
	void RadiusToLightspace(ShadowFrustum& frustum);

	EventBroker* m_EventBroker;
	const IRenderer* m_Renderer;

	GLuint m_DepthMap;
	FrameBuffer m_DepthBuffer;
    ShaderProgram* m_ShadowProgram;
    ShaderProgram* m_ShadowProgramSkinned;

	std::array<glm::mat4, MAX_SPLITS> m_LightProjection;
	std::array<glm::mat4, MAX_SPLITS> m_LightView;

	GLfloat m_NearFarPlane[2] = { -128.f, 128.f };
	GLuint m_ResolutionSizeWidth = 1024 * 2;
	GLuint m_ResolutionSizeHeight = 1024 * 2;

	bool m_TransparentObjects = false;
	bool m_TexturedShadows = false;
	bool m_EnableShadows = true;

	int m_CurrentNrOfSplits = 4;
	float m_SplitWeight = 0.7f;

	std::array<ShadowFrustum, MAX_SPLITS> m_shadowFrusta;

	Texture* m_WhiteTexture = ResourceManager::Load<Texture>("Textures/Core/White.png");
};

#endif