#ifndef PickingPass_h__
#define PickingPass_h__

#include "IRenderer.h"
#include "PickingPassState.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Util/UnorderedMapiVec2.h"
#include "../Core/EventBroker.h"
#include "../Core/World.h"
#include "Rendering/Skeleton.h"

class PickingPass
{
public:
    PickingPass(IRenderer* renderer, EventBroker* eb);
    ~PickingPass();
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();

    void Draw(RenderScene& scene);
    void ClearPicking();

    void OnWindowResize();

    //Getters
    const ShaderProgram& PickingProgram() const { return *m_PickingProgram; }
    //const std::unordered_map<glm::ivec2, EntityID>& PickingColorsToEntity() const { return m_PickingColorsToEntity; }
    GLuint PickingTexture() const { return m_PickingTexture; }
    GLuint* DepthBuffer() { return &m_DepthBuffer; }
    const FrameBuffer& PickingBuffer() const { return m_PickingBuffer; }

    
    PickData Pick(glm::vec2 screenCoord);

private:
    EventBroker* m_EventBroker;

    const IRenderer* m_Renderer;

    ShaderProgram* m_PickingProgram;
    ShaderProgram* m_PickingSkinnedProgram;
    Camera* m_Camera;

    struct PickingInfo
    {
        EntityID Entity;
        const ::World* World;
        ::Camera* Camera;
    };

    std::unordered_map<glm::ivec2, PickingInfo> m_PickingColorsToEntity;

    GLuint m_PickingTexture;
    GLuint m_DepthBuffer;

    FrameBuffer m_PickingBuffer;

    int m_ColorCounter[2];
    std::map<std::tuple<EntityID, const World*, Camera*>, glm::ivec2> m_EntityColors;
};

#endif 