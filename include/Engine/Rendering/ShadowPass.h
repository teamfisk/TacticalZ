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

//#include "ShadowPassState.h" // not created yet

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
    glm::mat4 lightSpaceMatrix() const { return m_LightSpaceMatrix; }
    glm::mat4 lightP() const { return m_LightProjection; }
    glm::mat4 lightV() const { return m_LightView; }

    void setResolution(GLuint width, GLuint height) { resolutionSizeWidth = width; resolutionSizeHeigth = height; }

private:

	glm::mat4 CalculateFrustum(RenderScene & scene, std::shared_ptr<DirectionalLightJob> directionalLightJob, glm::mat4& p, glm::mat4& v, ShadowCamera shad_cam);
	glm::vec3 LightDirectionToPoint(glm::vec4 direction);
	std::array<glm::vec3, 8> UpdateFrustumPoints(Camera* cam, glm::vec3 center, glm::vec3 view_dir);
	void UpdateSplitDist(std::array<ShadowCamera, MAX_SPLITS> shadow_cams, float far_distance, float near_distance);
	void InitializeLightCameras();
	glm::mat4 ApplyCropMatrix(ShadowCamera& shadow_cam, glm::mat4 m, glm::mat4 v);
	glm::mat4 FindNewFrustum(ShadowCamera shadow_cam);

    EventBroker* m_EventBroker;

    const IRenderer* m_Renderer;

	std::array<GLuint, MAX_SPLITS> m_DepthMap;
	std::array<FrameBuffer, MAX_SPLITS> m_DepthBuffer;

    ShaderProgram* m_ShadowProgram;

    GLuint m_DepthFBO;

    GLfloat m_NearFarPlane[2] = { -34.f, 27.f };
    //GLfloat m_LRBT[4] = { -77.f, 75.f, -89.f, 89.f };
	GLfloat m_LRBT[4] = { -10.f, 10.f, -10.f, 10.f };

    glm::mat4 m_LightProjection;
    glm::mat4 m_LightView;
    glm::mat4 m_LightSpaceMatrix;

    GLuint resolutionSizeWidth = 1024 * 2;
    GLuint resolutionSizeHeigth = 1024 * 2;

    bool m_ShadowOn = true;
	int m_ShadowLevel = 0;

	int m_CurrentNrOfSplits = 3;
	float m_SplitWeight = 0.75f;

	std::array<ShadowCamera, MAX_SPLITS> m_shadCams;
};

#endif