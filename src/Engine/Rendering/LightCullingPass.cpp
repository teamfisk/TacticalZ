#include "Rendering/LightCullingPass.h"

LightCullingPass::LightCullingPass(IRenderer* renderer)
{
    m_Renderer = renderer;
    SetSSBOSizes();
    InitializeSSBOs();
    InitializeShaderPrograms();
    //GenerateNewFrustum(TODO);
}

LightCullingPass::~LightCullingPass()
{

}

void LightCullingPass::GenerateNewFrustum(RenderScene& scene)
{
    if (scene.PointLightJobs.size() == 0)
        return;

    GLERROR("CalculateFrustum Error: Pre");

    m_CalculateFrustumProgram->Bind();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_FrustumSSBO);
    glUniformMatrix4fv(glGetUniformLocation(m_CalculateFrustumProgram->GetHandle(), "P"), 1, false, glm::value_ptr(scene.Camera->ProjectionMatrix()));
    glUniform2f(glGetUniformLocation(m_CalculateFrustumProgram->GetHandle(), "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);
    glDispatchCompute((int)(m_Renderer->Resolution().Width/(TILE_SIZE*TILE_SIZE) + 1), (int)(m_Renderer->Resolution().Height/(TILE_SIZE*TILE_SIZE) + 1), 1);

    GLERROR("CalculateFrustum Error: End");
}


void LightCullingPass::OnResolutionChange()
{
    SetSSBOSizes();
}


void LightCullingPass::SetSSBOSizes()
{
    m_NumberOfTiles = (int)(m_Renderer->Resolution().Width*m_Renderer->Resolution().Height)/TILE_SIZE;

    //m_Frustums = new Frustum[s];
    //m_LightGrid = new LightGrid[s];
    //m_LightIndex = new float[s*200];

    m_Frustums = new Frustum[m_NumberOfTiles];
    m_LightGrid = new LightGrid[m_NumberOfTiles];
    m_LightIndex = new float[m_NumberOfTiles*MAX_LIGHTS_PER_TILE];
}

void LightCullingPass::CullLights(RenderScene& scene)
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
    glUniform2f(glGetUniformLocation(m_LightCullProgram->GetHandle(), "ScreenDimensions"), m_Renderer->Resolution().Width, m_Renderer->Resolution().Height);
    glUniformMatrix4fv(glGetUniformLocation(m_LightCullProgram->GetHandle(), "V"), 1, false, glm::value_ptr(scene.Camera->ViewMatrix()));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_FrustumSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_LightSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightGridSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_LightOffsetSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_LightIndexSSBO);
    glDispatchCompute(m_Renderer->Resolution().Width / TILE_SIZE, m_Renderer->Resolution().Height / TILE_SIZE, 1);

    GLERROR("CullLights Error: End");
}

void LightCullingPass::FillLightList(RenderScene& scene)
{
    m_PointLights.clear();

    for(auto &job : scene.PointLightJobs) {
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
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Frustum)*m_NumberOfTiles, m_Frustums, GL_DYNAMIC_COPY);
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
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightGrid)*m_NumberOfTiles, m_LightGrid, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GLERROR("m_LightGridSSBO");


    glGenBuffers(1, &m_LightOffsetSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightOffsetSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_LightOffset), &m_LightOffset, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GLERROR("m_LightOffsetSSBO");

    glGenBuffers(1, &m_LightIndexSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightIndexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float)*m_NumberOfTiles*MAX_LIGHTS_PER_TILE, m_LightIndex, GL_DYNAMIC_COPY);
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

