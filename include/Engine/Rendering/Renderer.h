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
#include "DrawScenePass.h"
#include "DebugCameraInputController.h"
#include "LightCullingPass.h"
#include "DrawFinalPass.h"

#include "../Core/EventBroker.h"
#include "EPicking.h"
#include "ImGuiRenderPass.h"

class Renderer : public IRenderer
{
public:
    Renderer(EventBroker* eventBroker) 
        : m_EventBroker(eventBroker)
    { }

    virtual void Initialize() override;
    virtual void Update(double dt) override;
    virtual void Draw(RenderQueueCollection& rq) override;

private:
    //----------------------Variables----------------------//
    EventBroker* m_EventBroker;

    std::shared_ptr<DebugCameraInputController<Renderer>> m_DebugCameraInputController;

    Texture* m_ErrorTexture;
    Texture* m_WhiteTexture;
    float m_CameraMoveSpeed;

    Model* m_ScreenQuad;
    Model* m_UnitQuad;
    Model* m_UnitSphere;

    DrawScenePass* m_DrawScenePass;
    PickingPass* m_PickingPass;
    LightCullingPass* m_LightCullingPass;
    ImGuiRenderPass* m_ImGuiRenderPass;
    DrawFinalPass* m_DrawFinalPass;

    //----------------------Functions----------------------//
    void InitializeWindow();
    void InitializeShaders();
    void InitializeTextures();
    void InitializeRenderPasses();
    //TODO: Renderer: Get InputUpdate out of renderer
    void InputUpdate(double dt);
    //void PickingPass(RenderQueueCollection& rq);
    void DrawScreenQuad(GLuint textureToDraw);

    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type);
	//--------------------ShaderPrograms-------------------//
	ShaderProgram* m_BasicForwardProgram;
    ShaderProgram* m_DrawScreenQuadProgram;
};

#endif