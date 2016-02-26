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
#define MAP_SIZE 216.f

enum NearFar { Near = 0, Far = 1 };
enum LRBT { Left = 0, Right = 1, Bottom = 2, Top = 3 };

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
    ~ShadowPass();
    
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();
    void ClearBuffer();
    void Draw(RenderScene& scene);

    //GLuint DepthMap(int level) const { return m_DepthMap[level]; }
	GLuint DepthMap() const { return m_DepthMap; }
	std::array<glm::mat4, MAX_SPLITS> LightP() const { return m_LightProjection; }
	std::array<glm::mat4, MAX_SPLITS> LightV() const { return m_LightView; }
	std::array<float, MAX_SPLITS> FarDistance() const { return { m_shadowFrusta[0].FarClip, m_shadowFrusta[1].FarClip, m_shadowFrusta[2].FarClip, m_shadowFrusta[3].FarClip }; }
	int CurrentNrOfSplits() const { return m_CurrentNrOfSplits; }
private:
	void UpdateFrustumPoints(Frustum& frustum, glm::vec3 camera_position, glm::vec3 view_dir, glm::mat4 p, glm::mat4 v);
	void UpdateSplitDist(std::array<Frustum, MAX_SPLITS>& frusta, float near_distance, float far_distance);
	void InitializeCameras(RenderScene & scene);
	float FindRadius(Frustum& frustum);
	void PointsToLightspace(Frustum& frustum, glm::mat4 v);
	void RadiusToLightspace(Frustum& frustum, glm::mat4 v);
	
	EventBroker* m_EventBroker;
	const IRenderer* m_Renderer;

	//std::array<GLuint, MAX_SPLITS> m_DepthMap;
	//std::array<FrameBuffer, MAX_SPLITS> m_DepthBuffer;
	GLuint m_DepthMap;
	FrameBuffer m_DepthBuffer;

	std::array<glm::mat4, MAX_SPLITS> m_LightProjection;
	std::array<glm::mat4, MAX_SPLITS> m_LightView;

    ShaderProgram* m_ShadowProgram;
	GLuint m_DepthFBO;

    GLfloat m_NearFarPlane[2] = { -34.f, 27.f };
	GLfloat m_LRBT[4] = { -10.f, 10.f, -10.f, 10.f };

    GLuint m_ResolutionSizeWidth = 1024 * 2;
    GLuint m_ResolutionSizeHeigth = 1024 * 2;

    bool m_ShadowOn = true;
	int m_ShadowLevel = 0;

	int m_CurrentNrOfSplits = 3;
	float m_SplitWeight = 0.75f;

	std::array<Frustum, MAX_SPLITS> m_shadowFrusta;
};

#endif