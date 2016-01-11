#include "Rendering/LightCullingPass.h"

LightCullingPass::LightCullingPass(IRenderer* renderer)
{
    m_Renderer = renderer;
    InitializeSSBOs();
    InitializeShaderPrograms();
    GenerateNewFrustum();
}

LightCullingPass::~LightCullingPass()
{

}

void LightCullingPass::GenerateNewFrustum()
{
    GLERROR("CalculateFrustum Error: Pre");

    m_CalculateFrustumProgram->Bind();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_FrustumSSBO);
    glUniformMatrix4fv(glGetUniformLocation(m_CalculateFrustumProgram->GetHandle(), "P"), 1, false, glm::value_ptr(m_Renderer->Camera()->ProjectionMatrix()));
    glUniform2f(glGetUniformLocation(m_CalculateFrustumProgram->GetHandle(), "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);
    glDispatchCompute(5, 3, 1); //TODO: Renderer: This needs change so resolution will be right.

    GLERROR("CalculateFrustum Error: End");
}

void LightCullingPass::CullLights()
{
    GLERROR("CullLights Error: Pre");
    m_LightOffset = 0;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightOffsetSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_LightOffset), &m_LightOffset, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightSSBO);
    if (m_PointLights.size() > 0) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * m_PointLights.size(), &(m_PointLights[0]), GL_DYNAMIC_COPY);
    } else {
        GLfloat zero = 0.f;
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat), &zero , GL_DYNAMIC_COPY);

    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_LightCullProgram->Bind();
    glUniformMatrix4fv(glGetUniformLocation(m_LightCullProgram->GetHandle(), "V"), 1, false, glm::value_ptr(m_Renderer->Camera()->ViewMatrix()));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_FrustumSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightGridSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_LightOffsetSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightIndexSSBO);
    glDispatchCompute(m_Renderer->Resolution().Width / TILE_SIZE, m_Renderer->Resolution().Height / TILE_SIZE, 1);

    GLERROR("CullLights Error: End");
}

void LightCullingPass::FillLightList(RenderQueueCollection& rq)
{
    m_PointLights.clear();
    for(auto &job : rq.Lights) {
        auto pointLightjob = std::dynamic_pointer_cast<PointLightJob>(job);
        if (pointLightjob) {
            PointLight p;
            p.Color = pointLightjob->Color;
            p.Falloff = pointLightjob->Falloff;
            p.Intensity = pointLightjob->Intensity;
            p.Position = glm::vec4(glm::vec3(pointLightjob->Position), 1.f);
            p.Radius = pointLightjob->Radius;
            p.Padding = 123.f;
            m_PointLights.push_back(p);
            continue;
        }
    }
}

void LightCullingPass::InitializeSSBOs()
{
    glGenBuffers(1, &m_FrustumSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_FrustumSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_Frustums), &m_Frustums, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GLERROR("m_FrustumSSBO");

    glGenBuffers(1, &m_LightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightSSBO);
    if(m_PointLights.size() > 0) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * m_PointLights.size(), &(m_PointLights[0]), GL_DYNAMIC_COPY);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GLERROR("m_LightSSBO");

    glGenBuffers(1, &m_LightGridSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightGridSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_LightGrid), &m_LightGrid, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GLERROR("m_LightGridSSBO");


    glGenBuffers(1, &m_LightOffsetSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightOffsetSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_LightOffset), &m_LightOffset, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GLERROR("m_LightOffsetSSBO");

    glGenBuffers(1, &m_LightIndexSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightIndexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_LightIndex), &m_LightIndex, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GLERROR("m_LightIndexSSBO");
}

void LightCullingPass::InitializeShaderPrograms()
{
    m_CalculateFrustumProgram = ResourceManager::Load<ShaderProgram>("#CalculateFrustumProgram");
    m_CalculateFrustumProgram->AddShader(std::shared_ptr<Shader>(new ComputeShader("Shaders/GridFrustum.comp.glsl")));
    m_CalculateFrustumProgram->Compile();
    m_CalculateFrustumProgram->Link();

    m_LightCullProgram = ResourceManager::Load<ShaderProgram>("#LightCullProgram");
    m_LightCullProgram->AddShader(std::shared_ptr<Shader>(new ComputeShader("Shaders/CullLights.comp.glsl")));
    m_LightCullProgram->Compile();
    m_LightCullProgram->Link();
}

