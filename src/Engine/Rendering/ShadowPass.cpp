#include "Rendering/ShadowPass.h"

ShadowPass::ShadowPass(IRenderer * renderer)
{
    m_Renderer = renderer;

    //InitializeTextures();
    InitializeFrameBuffers();
    InitializeShaderPrograms();
}

ShadowPass::~ShadowPass()
{

}


void ShadowPass::InitializeFrameBuffers()
{
    
//     glGenRenderbuffers(1, &m_DepthFBO);
//     glBindRenderbuffer(GL_RENDERBUFFER, m_DepthFBO);
//     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);

    // Depth texture
    glGenTextures(1, &m_DepthMap);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolutionSizeWidth, resolutionSizeHeigth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_DepthBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_DepthMap, GL_DEPTH_ATTACHMENT)));
    //m_DepthBuffer.AddResource(std::shared_ptr<BufferResource>(new Texture2D(&m_DepthMap, GL_COLOR_ATTACHMENT0)));
    m_DepthBuffer.Generate();

    GLERROR("depthMap failed");

}

void ShadowPass::InitializeShaderPrograms()
{
    m_ShadowProgram = ResourceManager::Load<ShaderProgram>("#ShadowProgram");
    m_ShadowProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Shadow.vert.glsl")));
    m_ShadowProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Shadow.frag.glsl")));
    m_ShadowProgram->Compile();
    m_ShadowProgram->BindFragDataLocation(0, "ShadowMap");
    m_ShadowProgram->Link();
}

void ShadowPass::ClearBuffer()
{
    m_DepthBuffer.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_DepthBuffer.Unbind();
}

void ShadowPass::Draw(RenderScene & scene)
{
    ShadowPassState* state = new ShadowPassState(m_DepthBuffer.GetHandle());

    GLuint shaderHandle = m_ShadowProgram->GetHandle();
    m_ShadowProgram->Bind();

    glViewport(0, 0, resolutionSizeWidth, resolutionSizeHeigth);
    glCullFace(GL_FRONT);


    ImGui::DragFloat4("ShadowMapCam", m_LRBT, 1.f, -1000.f, 1000.f);
    ImGui::DragFloat2("ShadowMapNearFar", m_NearFarPlane, 1.f, -1000.f, 1000.f);

    for (auto &job : scene.DirectionalLightJobs) {
        auto directionalLightJob = std::dynamic_pointer_cast<DirectionalLightJob>(job);
        
        if(directionalLightJob) {
            //m_LightProjection = glm::ortho(m_LRBT[Left], m_LRBT[Right], m_LRBT[Bottom], m_LRBT[Top], m_NearFarPlane[Near], m_NearFarPlane[Far]);
            m_LightProjection = glm::ortho(m_LRBT[Left], m_LRBT[Right], m_LRBT[Bottom], m_LRBT[Top], m_NearFarPlane[Near], m_NearFarPlane[Far]);
            m_LightView = glm::lookAt(-glm::normalize(glm::vec3(directionalLightJob->Direction)), glm::vec3(0.f,0.f,0.f), glm::vec3(0.f,1.f,0.f));
            //m_LightView = glm::lookAt(glm::vec3(-20.0f, 20.0f, -20.0f), glm::vec3(0.0f), glm::vec3(1.0));
            m_LightSpaceMatrix = m_LightProjection * m_LightView;

            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(m_LightProjection));
            glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(m_LightView));

            GLERROR("ShadowLight ERROR");

            for (auto &objectJob : scene.OpaqueObjects) {
                auto modelJob = std::dynamic_pointer_cast<ModelJob>(objectJob);
                
                glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));

                glBindVertexArray(modelJob->Model->VAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
                glDrawElements(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, (void*)(modelJob->StartIndex * sizeof(unsigned int)));

                GLERROR("Shadow Draw ERROR");

            }
          
        }

        m_DepthBuffer.Unbind();

        delete state;


    }



    glViewport(0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height);
    glCullFace(GL_BACK);
    m_ShadowProgram->Unbind();





}
