#ifndef LightCullingPass_h__
#define LightCullingPass_h__

#define TILE_SIZE 16 
#define MAX_LIGHTS_PER_TILE 200

#include "IRenderer.h"
#include "LightCullingPassState.h"
#include "ShaderProgram.h"
#include "RenderQueue.h"


class LightCullingPass
{
public:
    LightCullingPass(IRenderer* renderer);
    ~LightCullingPass();

    void GenerateNewFrustum(RenderScene& scene);
    void OnResolutionChange();
    void SetSSBOSizes();
    void CullLights(RenderScene& scene);
    void FillLightList(RenderScene& scene);
    void OnWindowResize();

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

    int m_NumberOfTiles = 0;

    struct Plane {
        glm::vec3 Normal = glm::vec3(0.f);
        float d = 0;
    };

    struct Frustum {
        Plane Planes[4];
    };
    Frustum* m_Frustums = nullptr;

    //This should be a component
    struct LightSource {
        glm::vec4 Position = glm::vec4(0.f);
        glm::vec4 Direction = glm::vec4(10.f);
        glm::vec4 Color = glm::vec4(1.f);
        float Radius = 5.f;
        float Intensity = 0.8f;
        float Falloff = 0.3f;
        enum Type_t { Zero, Point, Directional, Spot } Type;
    };
    std::vector<LightSource> m_LightSources;

    struct LightGrid {
        float Start = 0;
        float Amount = 0;
        glm::vec2 Padding = glm::vec2(1.f, 2.f);
    };

    LightGrid* m_LightGrid = nullptr;

    int m_LightOffset = 0;

    float* m_LightIndex = nullptr;
};


#endif