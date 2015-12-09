#ifndef Renderer_h__
#define Renderer_h__

#include <sstream>

#include "IRenderer.h"
#include "ShaderProgram.h"
//TODO: Temp resourceManager
#include "../Core/ResourceManager.h"
#include "Util/UnorderedMapVec2.h"
#include "FrameBuffer.h"
#include "../Core/World.h"

#define TILE_SIZE 16 
#define NUM_LIGHTS 3


enum lightType 
{
    Point,
    Spot,
    Directional,
    Area
};

class Renderer : public IRenderer
{
public:
    virtual void Initialize() override;
    virtual void Update(double dt) override;
    virtual void Draw(RenderQueueCollection& rq) override;

private:
    //----------------------Variables----------------------//
    Texture* m_ErrorTexture;
    Texture* m_WhiteTexture;
    float m_CameraMoveSpeed;
    FrameBuffer m_PickingBuffer;
    GLuint m_PickingTexture;
    GLuint m_DepthBuffer;

    Model* m_ScreenQuad;
    Model* m_UnitQuad;
    Model* m_UnitSphere;



    std::unordered_map<glm::vec2, EntityID> m_PickingColorsToEntity;

    //----------------------Functions----------------------//
    void InitializeWindow();
    void InitializeShaders();
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeSSBOs();
    //TODO: Renderer: Get InputUpdate out of renderer
    void InputUpdate(double dt);
    void PickingPass(RenderQueueCollection& rq);
    void DrawScreenQuad(GLuint textureToDraw);
    void DrawScene(RenderQueueCollection& rq);

    //----------------------Forward+-----------------------//
    void CalculateFrustum();
    void CullLights();
    //Frustum
    struct Plane
    {
        glm::vec3 Normal;
        float d;
    };

    struct Frustum
    {
        Plane plane[4];
    };
    Frustum m_Frustums[80*45]; //TODO: Renderer: Make this change with resolution

    //Lights
    void TEMPCreateLights();
    //TODO: Renderer: Add Directionllights, spotlights and area lights to this as type.
    struct PointLight {
        glm::vec4 Position = glm::vec4(0.f);
        glm::vec4 Color = glm::vec4(1.f);
        float Radius = 5.f;
        float Intensity = 0.8f;
        float Falloff = 0.3f;
        float Padding = 1337;
    };
    PointLight m_PointLights[NUM_LIGHTS];

    struct LightGrid {
        int Amount;
        int Start;
        glm::vec2 Padding;
    };
    LightGrid m_LightGrid[80*45];

    int m_LightOffset = 0;

    int m_LightIndex[80*45*200];

    //-------------------------SSBO------------------------//
    GLuint m_FrustumSSBO;
    GLuint m_LightSSBO;
    GLuint m_LightGridSSBO;
    GLuint m_LightOffsetSSBO;
    GLuint m_LightIndexSSBO;

    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type);
	//--------------------ShaderPrograms-------------------//
	ShaderProgram m_BasicForwardProgram;
    ShaderProgram m_PickingProgram;
    ShaderProgram m_DrawScreenQuadProgram;
    ShaderProgram m_CalculateFrustumProgram;
    ShaderProgram m_LightCullProgram;

};

#endif