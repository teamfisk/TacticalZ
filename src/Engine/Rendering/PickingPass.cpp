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
	CommonFunctions::DeleteTexture(&m_PickingTexture);
	CommonFunctions::DeleteTexture(&m_DepthBuffer);
}



void PickingPass::InitializeTextures()
{
    CommonFunctions::GenerateTexture(&m_PickingTexture, GL_CLAMP_TO_BORDER, GL_LINEAR,
        glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_RG8, GL_RG, GL_UNSIGNED_BYTE);

	CommonFunctions::GenerateTexture(&m_DepthBuffer, GL_CLAMP_TO_BORDER, GL_NEAREST,
		glm::vec2(m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height), GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
}

void PickingPass::InitializeFrameBuffers()
{

    m_PickingBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_DepthBuffer, GL_DEPTH_ATTACHMENT)));
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

    m_PickingSkinnedProgram = ResourceManager::Load<ShaderProgram>("#PickingSkinnedProgram");

    m_PickingSkinnedProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/PickingSkinned.vert.glsl")));
    m_PickingSkinnedProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Picking.frag.glsl")));
    m_PickingSkinnedProgram->Compile();
    m_PickingSkinnedProgram->BindFragDataLocation(0, "TextureFragment");
    m_PickingSkinnedProgram->Link();
}

void PickingPass::Draw(RenderScene& scene)
{
    GLERROR("PRE");
    PickingPassState* state = new PickingPassState(m_PickingBuffer.GetHandle());

    //TODO: Render: Add code for more jobs than modeljobs.
    GLuint shaderHandle = m_PickingProgram->GetHandle();
    GLuint shaderSkinnedHandle = m_PickingSkinnedProgram->GetHandle();
    m_PickingProgram->Bind();

    if (scene.ClearDepth) {
        //glClear(GL_DEPTH_BUFFER_BIT);
		state->Disable(GL_DEPTH_TEST);
		state->DepthMask(GL_FALSE);
    }
    m_Camera = scene.Camera;

    for (auto &job : scene.Jobs.OpaqueObjects) {
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
                    m_ColorCounter[1] += 1;
                } else {
                    m_ColorCounter[0] += 1;
                }
            }

            m_PickingColorsToEntity[glm::ivec2(pickColor[0], pickColor[1])] = pickInfo;

            if (modelJob->Model->IsSkinned()) {
                m_PickingSkinnedProgram->Bind();
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
                glUniform2fv(glGetUniformLocation(shaderSkinnedHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));

                std::vector<glm::mat4> frameBones;
				if (modelJob->BlendTree != nullptr) {
                    frameBones = modelJob->BlendTree->GetFinalPose();
                } else {
                    frameBones = modelJob->Skeleton->GetTPose();
                }
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));


            } else {
                m_PickingProgram->Bind();
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
                glUniform2fv(glGetUniformLocation(shaderHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));
            }

            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));
        }
    }

   /* for (auto &job : scene.Jobs.TransparentObjects) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);

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
                m_ColorCounter[1] += 1;
            } else {
                m_ColorCounter[0] += 1;
            }
        }

        m_PickingColorsToEntity[glm::ivec2(pickColor[0], pickColor[1])] = pickInfo;

        if (modelJob) {
            if (modelJob->Model->IsSkinned()) {
                m_PickingSkinnedProgram->Bind();
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
                glUniform2fv(glGetUniformLocation(shaderSkinnedHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));

                if (modelJob->BlendTree != nullptr) {
                    std::vector<glm::mat4> frameBones;
                    frameBones = modelJob->BlendTree->GetFinalPose();
                    glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
                }
            } else {
                m_PickingProgram->Bind();
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
                glUniform2fv(glGetUniformLocation(shaderHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));
            }

            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));
        }
    }*/

    for (auto &job : scene.Jobs.OpaqueShieldedObjects) {
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
                    m_ColorCounter[1] += 1;
                } else {
                    m_ColorCounter[0] += 1;
                }
            }

            m_PickingColorsToEntity[glm::ivec2(pickColor[0], pickColor[1])] = pickInfo;

            if (modelJob->Model->IsSkinned()) {
                m_PickingSkinnedProgram->Bind();
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
                glUniform2fv(glGetUniformLocation(shaderSkinnedHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));

                std::vector<glm::mat4> frameBones;
                if (modelJob->BlendTree != nullptr) {
                    frameBones = modelJob->BlendTree->GetFinalPose();
                } else {
                    frameBones = modelJob->Skeleton->GetTPose();
                }

                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));

            } else {
                m_PickingProgram->Bind();
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
                glUniform2fv(glGetUniformLocation(shaderHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));
            }

            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));
        }
    }

    for (auto& job : scene.Jobs.SpriteJob) {
        auto spriteJob = std::dynamic_pointer_cast<SpriteJob>(job);
        if (!spriteJob->Pickable) {
            continue;
        }
        RenderState jobState;

        if (spriteJob) {
            if (spriteJob->Depth == 0) {
                jobState.Disable(GL_DEPTH_TEST);
            }
            int pickColor[2] = { m_ColorCounter[0], m_ColorCounter[1] };

            PickingInfo pickInfo;
            pickInfo.Entity = spriteJob->Entity;
            pickInfo.World = spriteJob->World;
            pickInfo.Camera = scene.Camera;

            auto color = m_EntityColors.find(std::make_tuple(pickInfo.Entity, pickInfo.World, pickInfo.Camera));
            if (color != m_EntityColors.end()) {
                pickColor[0] = color->second[0];
                pickColor[1] = color->second[1];
            } else {
                m_EntityColors[std::make_tuple(pickInfo.Entity, pickInfo.World, pickInfo.Camera)] = glm::ivec2(pickColor[0], pickColor[1]);
                if (m_ColorCounter[0] > 255) {
                    m_ColorCounter[0] = 0;
                    m_ColorCounter[1] += 1;
                } else {
                    m_ColorCounter[0] += 1;
                }
            }

            m_PickingColorsToEntity[glm::ivec2(pickColor[0], pickColor[1])] = pickInfo;

            m_PickingProgram->Bind();
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(spriteJob->Matrix));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
            glUniform2fv(glGetUniformLocation(shaderHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));

            glBindVertexArray(spriteJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, spriteJob->EndIndex - spriteJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(spriteJob->StartIndex * sizeof(unsigned int)));
        }
    }

 /*   for (auto &job : scene.Jobs.TransparentShieldedObjects) {
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
                    m_ColorCounter[1] += 1;
                } else {
                    m_ColorCounter[0] += 1;
                }
            }

            m_PickingColorsToEntity[glm::ivec2(pickColor[0], pickColor[1])] = pickInfo;

            if (modelJob->Model->IsSkinned()) {
                m_PickingSkinnedProgram->Bind();


                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
                glUniform2fv(glGetUniformLocation(shaderSkinnedHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));

                if (modelJob->BlendTree != nullptr) {
                    std::vector<glm::mat4> frameBones;
                    frameBones = modelJob->BlendTree->GetFinalPose();
                    glUniformMatrix4fv(glGetUniformLocation(shaderSkinnedHandle, "Bones"), frameBones.size(), GL_FALSE, glm::value_ptr(frameBones[0]));
                }
            } else {
                m_PickingProgram->Bind();

                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));
                glUniform2fv(glGetUniformLocation(shaderHandle, "PickingColor"), 1, glm::value_ptr(glm::vec2(pickColor[0], pickColor[1])));
            }




            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));
        }
    }*/

    m_PickingBuffer.Unbind();
    GLERROR("PickingPass Error");

    delete state;
}

void PickingPass::ClearPicking()
{
    GLERROR("PRE");
    m_PickingColorsToEntity.clear();
    m_EntityColors.clear();
    m_ColorCounter[0] = 0;
    m_ColorCounter[1] = 1;

    m_PickingBuffer.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    m_PickingBuffer.Unbind();
    GLERROR("END");
}


void PickingPass::OnWindowResize()
{
    InitializeTextures();
    m_PickingBuffer.Generate();
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
