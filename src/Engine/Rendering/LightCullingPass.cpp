#include "Rendering/LightCullingPass.h"

LightCullingPass::LightCullingPass(IRenderer* renderer)
{
    m_Renderer = renderer;
    TEMPCreateLights();
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

void LightCullingPass::InitializeSSBOs()
{
    glGenBuffers(1, &m_FrustumSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_FrustumSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_Frustums), &m_Frustums, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GLERROR("m_FrustumSSBO");

    glGenBuffers(1, &m_LightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_PointLights), &m_PointLights, GL_DYNAMIC_COPY);
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
    m_LightCullProgram->AddShader(std::shared_ptr<Shader>(new ComputeShader("Shaders/cullLights.comp.glsl")));
    m_LightCullProgram->Compile();
    m_LightCullProgram->Link();
}

void LightCullingPass::TEMPCreateLights()
{
    for (int i = 0; i < NUM_LIGHTS; i++) {
        glm::vec3 pos = glm::vec3(cos(i) * i/10.f, 0.5f, sin(i) * i/10.f);
        m_PointLights[i].Position = glm::vec4(pos, 1.f);
        m_PointLights[i].Color = glm::vec4(rand()%255 / 255.f, rand()%255 / 255.f, rand()%255 / 255.f, 1.f);
        m_PointLights[i].Radius = glm::length(pos) / 5.f;
    }
}
