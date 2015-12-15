#include "Rendering/PickingPass.h"

PickingPass::PickingPass(IRenderer* renderer, EventBroker* eb)
{
    m_Renderer = renderer;
    m_EventBroker = eb;

    InitializeTextures();
    InitializeFrameBuffers();
    InitializeShaderPrograms();
}

PickingPass::~PickingPass()
{

}

void PickingPass::InitializeTextures()
{
    GenerateTexture(&m_PickingTexture, GL_CLAMP_TO_BORDER, GL_LINEAR,
        glm::vec2(m_Renderer->Resolution().Width, m_Renderer->Resolution().Height), GL_RG8, GL_RG, GL_UNSIGNED_BYTE);
}

void PickingPass::InitializeFrameBuffers()
{
    glGenRenderbuffers(1, &m_DepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);

    m_PickingBuffer.AddResource(std::shared_ptr<BufferResource>(new RenderBuffer(&m_DepthBuffer, GL_DEPTH_ATTACHMENT)));
    m_PickingBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_PickingTexture, GL_COLOR_ATTACHMENT0)));
    m_PickingBuffer.Generate();
}

void PickingPass::InitializeShaderPrograms()
{
    m_PickingProgram = ResourceManager::Load<ShaderProgram>("#PickingProgram");

    m_PickingProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Picking.vert.glsl")));
    m_PickingProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Picking.frag.glsl")));
    m_PickingProgram->Compile();
    m_PickingProgram->BindFragDataLocation(0, "TextureFragment");
    m_PickingProgram->Link();
}

void PickingPass::Draw(RenderQueueCollection& rq)
{
    m_PickingColorsToEntity.clear();
    PickingPassState* state = new PickingPassState(m_PickingBuffer.GetHandle());

    int r = 1;
    int g = 0;
    //TODO: Render: Add code for more jobs than modeljobs.

    GLuint ShaderHandle = m_PickingProgram->GetHandle();
    m_PickingProgram->Bind();

    for (auto &job : rq.Forward) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);

        if (modelJob) {
            //---------------
            //TODO: Renderer: IMPORTANT: Fixa detta s� det inte loopar igenom listan varje frame.
            //---------------
            int pickColor[2] = { r, g };
            for (auto i : m_PickingColorsToEntity) {
                if (modelJob->Entity == i.second) {
                    pickColor[0] = i.first.x;
                    pickColor[1] = i.first.y;
                    r -= 1;
                }
            }
            m_PickingColorsToEntity[glm::vec2(pickColor[0], pickColor[1])] = modelJob->Entity;

            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "Matrix"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
            glUniform2fv(glGetUniformLocation(ShaderHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));

            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElementsBaseVertex(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, nullptr, modelJob->StartIndex);
            r += 1;
            if (r > 255) {
                r = 0;
                g += 1;
            }
        }
    }
    m_PickingBuffer.Unbind();
    GLERROR("PickingPass Error");

    //Publish pick event every frame with the pick data that can be picked by the event
    int fbWidth;
    int fbHeight;
    glfwGetFramebufferSize(m_Renderer->Window(), &fbWidth, &fbHeight);
  /*  Events::Picking pickEvent = Events::Picking(
        &m_PickingBuffer,
        &m_DepthBuffer,
        m_Renderer->Camera()->ProjectionMatrix(),
        m_Renderer->Camera()->ViewMatrix(),
        Rectangle(fbWidth, fbHeight),
        &m_PickingColorsToEntity);*/

    //m_EventBroker->Publish(pickEvent);
}

void PickingPass::GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const
{
    //TODO: Renderer: Make this in a sparate class
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, dimensions.x, dimensions.y, 0, format, type, nullptr);//TODO: Renderer: Fix the precision and Resolution
    GLERROR("Texture initialization failed");
}
