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
#include "../Core/EventBroker.h"
#include "ImGuiRenderPass.h"
#include "Camera.h"

class Renderer : public IRenderer
{
public:
    Renderer(EventBroker* eventBroker, World* world) 
        : m_EventBroker(eventBroker)
        , m_World(world)
    { }

    virtual void Initialize() override;
    virtual void Update(double dt) override;
    virtual void Draw(RenderFrame& frame) override;

    virtual PickData Pick(glm::vec2 screenCoord) override;

private:
    //----------------------Variables----------------------//
    EventBroker* m_EventBroker;
    World* m_World;

    Texture* m_ErrorTexture;
    Texture* m_WhiteTexture;

    Model* m_ScreenQuad;
    Model* m_UnitQuad;
    Model* m_UnitSphere;

    DrawScenePass* m_DrawScenePass;
    PickingPass* m_PickingPass;
    ImGuiRenderPass* m_ImGuiRenderPass;

    //----------------------Functions----------------------//
    void InitializeWindow();
    void InitializeShaders();
    void InitializeTextures();
    void InitializeSSBOs();
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