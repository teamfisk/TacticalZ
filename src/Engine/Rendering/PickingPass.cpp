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
        glm::vec2(m_Renderer->GetViewPortSize().Width, m_Renderer->GetViewPortSize().Height), GL_RG8, GL_RG, GL_UNSIGNED_BYTE);
}

void PickingPass::InitializeFrameBuffers()
{
    glGenRenderbuffers(1, &m_DepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Renderer->GetViewPortSize().Width, m_Renderer->GetViewPortSize().Height);

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

void PickingPass::Draw(RenderScene& scene)
{
    PickingPassState* state = new PickingPassState(m_PickingBuffer.GetHandle());
    
    //TODO: Render: Add code for more jobs than modeljobs.
    GLuint shaderHandle = m_PickingProgram->GetHandle();
    m_PickingProgram->Bind();

    if (scene.ClearDepth) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    m_Camera = scene.Camera;

    for (auto &job : scene.ForwardJobs) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);

        if (modelJob) {
            int pickColor[2] = { m_ColorCounter[0], m_ColorCounter[1] };

            PickingInfo pickInfo;
            pickInfo.Entity = modelJob->Entity;
            pickInfo.World = modelJob->World;
            pickInfo.Camera = scene.Camera;

            auto color = m_EntityColors.find(std::make_tuple(pickInfo.Entity, pickInfo.World, pickInfo.Camera));
            if (color != m_EntityColors.end()) {
                pickColor[0] = color->second[0];
                pickColor[1] = color->second[1];
            } else {
                m_EntityColors[std::make_tuple(pickInfo.Entity, pickInfo.World, pickInfo.Camera)] = glm::ivec2(pickColor[0], pickColor[1]);
                if (m_ColorCounter[0] > 255) {
                    m_ColorCounter[0] = 0;
                        m_ColorCounter[1]++;
                } else {
                        m_ColorCounter[0]++;
                }
            }

            m_PickingColorsToEntity[glm::ivec2(pickColor[0], pickColor[1])] = pickInfo;

            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
            glUniform2fv(glGetUniformLocation(shaderHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));

            if (modelJob->Model->m_RawModel->m_Skeleton != nullptr) {

                if (modelJob->Animation != nullptr) {
                    std::vector<glm::mat4> frameBones = modelJob->Skeleton->GetFrameBones(*modelJob->Animation, modelJob->AnimationTime);
                    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
                }
            }

            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));
        }
    }
    
    m_PickingBuffer.Unbind();
    GLERROR("PickingPass Error");

    delete state;
}



void PickingPass::ClearPicking()
{
    m_PickingColorsToEntity.clear();
    m_EntityColors.clear();
    m_ColorCounter[0] = 1;
    m_ColorCounter[1] = 0;

    m_PickingBuffer.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_PickingBuffer.Unbind();
}

PickData PickingPass::Pick(glm::vec2 screenCoord)
{
    int fbWidth;
    int fbHeight;
    glfwGetFramebufferSize(m_Renderer->Window(), &fbWidth, &fbHeight);

    Rectangle resolution = Rectangle(fbWidth, fbHeight);
    PickData pickData;
    // Invert screen y coordinate
    screenCoord.y = resolution.Height - screenCoord.y;
    ScreenCoords::PixelData data = ScreenCoords::ToPixelData(screenCoord, &m_PickingBuffer, m_DepthBuffer);
    pickData.Depth = data.Depth;

    PickingInfo pickInfo;

    auto it = m_PickingColorsToEntity.find(glm::ivec2(data.Color[0], data.Color[1]));
    if (it != m_PickingColorsToEntity.end()) {
        pickInfo = it->second;
    } else {
        pickData.Entity = EntityID_Invalid;
        return pickData;
    }

    pickData.Position = ScreenCoords::ToWorldPos(screenCoord.x, screenCoord.y, data.Depth, resolution, pickInfo.Camera->ProjectionMatrix(), pickInfo.Camera->ViewMatrix());
    pickData.Entity = pickInfo.Entity;
    pickData.Camera = pickInfo.Camera;
    pickData.World = pickInfo.World;
    return pickData;
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
