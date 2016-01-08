#ifndef LightCullingPass_h__
#define LightCullingPass_h__

#define TILE_SIZE 16 
#define NUM_LIGHTS 1000

#include "IRenderer.h"
#include "LightCullingPassState.h"
#include "ShaderProgram.h"


class LightCullingPass
{
public:
    LightCullingPass(IRenderer* renderer);
    ~LightCullingPass();

    void GenerateNewFrustum();
    void CullLights();
    void FillLightList(RenderQueueCollection& rq);

    GLuint FrustumSSBO() const { return m_FrustumSSBO; }
    GLuint LightSSBO() const { return m_LightSSBO; }
    GLuint LightGridSSBO() const { return m_LightGridSSBO; }
    GLuint LightOffsetSSBO() const { return m_LightOffsetSSBO; }
    GLuint LightIndexSSBO() const { return m_LightIndexSSBO; }
private:

    void InitializeSSBOs();
    void InitializeShaderPrograms();

    const IRenderer* m_Renderer;

    GLuint m_FrustumSSBO = 0;
    GLuint m_LightSSBO = 0;
    GLuint m_LightGridSSBO = 0;
    GLuint m_LightOffsetSSBO = 0;
    GLuint m_LightIndexSSBO = 0;

    ShaderProgram* m_CalculateFrustumProgram;
    ShaderProgram* m_LightCullProgram;

    struct Plane {
        glm::vec3 Normal;
        float d;
    };

    struct Frustum {
        Plane Planes[4];
    };
    Frustum m_Frustums[80*45]; //TODO: Renderer: Make this change with resolution

    //This should be a component
    struct PointLight {
        glm::vec4 Position = glm::vec4(0.f);
        glm::vec4 Color = glm::vec4(1.f);
        float Radius = 5.f;
        float Intensity = 0.8f;
        float Falloff = 0.3f;
        float Padding = 1337;
    };
    std::vector<PointLight> m_PointLights;

    struct LightGrid {
        float Start;
        float Amount;
        glm::vec2 Padding;
    };

    LightGrid m_LightGrid[80*45]; //TODO: Renderer: Make this change with resolution

    int m_LightOffset = 0;

    float m_LightIndex[80*45*200]; //TODO: Renderer: Make this change with resolution
};


#endif