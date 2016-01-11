#ifndef PickingPass_h__
#define PickingPass_h__

#include "IRenderer.h"
#include "PickingPassState.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Util/UnorderedMapVec2.h"
#include "../Core/EventBroker.h"
#include "EPicking.h"

class PickingPass
{
public:
    PickingPass(IRenderer* renderer, EventBroker* eb);
    ~PickingPass();
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();

    void Draw(RenderQueueCollection& rq);


    //Getters
    const ShaderProgram& PickingProgram() const { return *m_PickingProgram; }
    const std::unordered_map<glm::vec2, EntityID>& PickingColorsToEntity() const { return m_PickingColorsToEntity; }
    GLuint PickingTexture() const { return m_PickingTexture; }
    GLuint DepthBuffer() const { return m_DepthBuffer; }
    const FrameBuffer& PickingBuffer() const { return m_PickingBuffer; }


private:
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const;

    EventBroker* m_EventBroker;

    const IRenderer* m_Renderer;

    ShaderProgram* m_PickingProgram;

    std::unordered_map<glm::vec2, EntityID> m_PickingColorsToEntity;

    GLuint m_PickingTexture;
    GLuint m_DepthBuffer;

    FrameBuffer m_PickingBuffer;
};

#endif 