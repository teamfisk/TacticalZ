#ifndef ShadowPass_h_
#define ShadowPass_h_

#include "IRenderer.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "../Core/EventBroker.h"
#include "../Core/World.h"
#include "ShadowPassState.h"
#include "imgui/imgui.h"

#define MAX_SPLITS 5

enum NearFar { Near = 0, Far = 1 };
enum LRBT { Left = 0, Right = 1, Bottom = 2, Top = 3 };

struct ShadowCamera{
	Camera* camera;
	std::array<glm::vec3, 8> frustumCorners;
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

    GLuint DepthMap() const { return m_DepthMap[m_ShadowLevel]; }
    glm::mat4 lightP() const { return m_LightProjection[m_ShadowLevel]; }
    glm::mat4 lightV() const { return m_LightView[m_ShadowLevel]; }
private:

	glm::mat4 CalculateFrustum(RenderScene & scene, std::shared_ptr<DirectionalLightJob> directionalLightJob, ShadowCamera shad_cam);
	std::array<glm::vec3, 8> UpdateFrustumPoints(Camera* cam, glm::vec3 center, glm::vec3 view_dir);
	void UpdateSplitDist(std::array<ShadowCamera, MAX_SPLITS> shadow_cams, float far_distance, float near_distance);
	glm::mat4 FindNewFrustum(ShadowCamera shadow_cam);

    EventBroker* m_EventBroker;
	const IRenderer* m_Renderer;

	std::array<GLuint, MAX_SPLITS> m_DepthMap;
	std::array<FrameBuffer, MAX_SPLITS> m_DepthBuffer;

	std::array<glm::mat4, MAX_SPLITS> m_LightProjection;
	std::array<glm::mat4, MAX_SPLITS> m_LightView;

    ShaderProgram* m_ShadowProgram;
	GLuint m_DepthFBO;

    GLfloat m_NearFarPlane[2] = { -34.f, 27.f };
	GLfloat m_LRBT[4] = { -10.f, 10.f, -10.f, 10.f };

    GLuint resolutionSizeWidth = 1024 * 2;
    GLuint resolutionSizeHeigth = 1024 * 2;

    bool m_ShadowOn = true;
	int m_ShadowLevel = 0;

	int m_CurrentNrOfSplits = 3;
	float m_SplitWeight = 0.5f;

	std::array<ShadowCamera, MAX_SPLITS> m_shadCams;
};

#endif