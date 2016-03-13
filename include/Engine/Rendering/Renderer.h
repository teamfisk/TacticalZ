#ifndef Renderer_h__
#define Renderer_h__

#include <sstream>

#include "IRenderer.h"
#include "ShaderProgram.h"
//TODO: Temp resourceManager
#include "../Core/ResourceManager.h"
#include "Util/UnorderedMapVec2.h"
#include "FrameBuffer.h"
#include "../Core/World.h"
#include "PickingPass.h"
#include "LightCullingPass.h"
#include "DrawFinalPass.h"
#include "DrawScreenQuadPass.h"
#include "DrawBloomPass.h"
#include "DrawColorCorrectionPass.h"
#include "SSAOPass.h"
#include "CubeMapPass.h"
#include "BlurHUD.h"
#include "../Core/EventBroker.h"
#include "ImGuiRenderPass.h"
#include "Camera.h"
#include "../Core/TransformSystem.h"
#include "imgui/imgui.h"
#include "TextPass.h"
#include "Util/CommonFunctions.h"
#include "Core/PerformanceTimer.h"
#include "ShadowPass.h"
#include "EResolutionChanged.h"

class Renderer : public IRenderer
{
    static void glfwFrameBufferCallback(GLFWwindow* window, int width, int height);
    static void glfwWindowSizeCallback(GLFWwindow* window, int width, int height);

public:
	Renderer(EventBroker* eventBroker, ConfigFile* config)
		: m_EventBroker(eventBroker)
		, m_Config(config)
    { }
	~Renderer();

    virtual void SetResolution(const Rectangle& resolution) override;

    virtual void Initialize() override;
    virtual void Update(double dt) override;
    virtual void Draw(RenderFrame& frame) override;

    virtual PickData Pick(glm::vec2 screenCoord) override;

private:
    //----------------------Variables----------------------//

    static std::unordered_map <GLFWwindow*, Renderer*> m_WindowToRenderer;
	ConfigFile* m_Config;

    EventBroker* m_EventBroker;
    TextPass* m_TextPass;

    Texture* m_ErrorTexture;
    Texture* m_WhiteTexture;

    Model* m_ScreenQuad;
    Model* m_UnitQuad;
    Model* m_UnitSphere;

    int m_DebugTextureToDraw = 0;
    int m_CubeMapTexture = 0;
    bool m_ResizeWindow = false;
	int m_SSAO_Quality = 0;
	int m_GLOW_Quality = 2;
	bool m_Shadow_Enabled = true;

    PickingPass* m_PickingPass;
    LightCullingPass* m_LightCullingPass;
    ImGuiRenderPass* m_ImGuiRenderPass;
    DrawFinalPass* m_DrawFinalPass;
    DrawScreenQuadPass* m_DrawScreenQuadPass;
    DrawBloomPass* m_DrawBloomPass;
    DrawColorCorrectionPass* m_DrawColorCorrectionPass;
	SSAOPass* m_SSAOPass;
    CubeMapPass* m_CubeMapPass;
	ShadowPass* m_ShadowPass;
    BlurHUD* m_BlurHUDPass;

    //----------------------Functions----------------------//
    void InitializeWindow();
    void InitializeShaders();
    void InitializeTextures();
    void InitializeRenderPasses();
    //TODO: Renderer: Get InputUpdate out of renderer
    void InputUpdate(double dt);
    //void PickingPass(RenderQueueCollection& rq);
    //void DrawScreenQuad(GLuint textureToDraw);
    void setWindowSize(Rectangle size);
    void updateFramebufferSize();

    static bool DepthSort(const std::shared_ptr<RenderJob> &i, const std::shared_ptr<RenderJob> &j) { return (i->Depth < j->Depth); }
    void SortRenderJobsByDepth(RenderScene &scene);
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type);

    //--------------------ShaderPrograms-------------------//
    ShaderProgram* m_BasicForwardProgram;
    ShaderProgram* m_ExplosionEffectProgram;

};

#endif