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
#include "PickingPass.h"


#define TILE_SIZE 16 
#define NUM_LIGHTS 3


enum lightType 
{
    Point,
    Spot,
    Directional,
    Area
};

#include "../Core/EventBroker.h"
#include "EPicking.h"

class Renderer : public IRenderer
{
public:
    Renderer(EventBroker* eventBroker) 
        : m_EventBroker(eventBroker)
    { }

    virtual void Initialize() override;
    virtual void Update(double dt) override;
    virtual void Draw(RenderQueueCollection& rq) override;

private:
    //----------------------Variables----------------------//
    EventBroker* m_EventBroker;

    Texture* m_ErrorTexture;
    Texture* m_WhiteTexture;
    float m_CameraMoveSpeed;

    Model* m_ScreenQuad;
    Model* m_UnitQuad;
    Model* m_UnitSphere;

    PickingPass* m_PickingPass;

    //----------------------Functions----------------------//
    void InitializeWindow();
    void InitializeShaders();
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeSSBOs();
    void InitializeRenderPasses();
    //TODO: Renderer: Get InputUpdate out of renderer
    void InputUpdate(double dt);
    //void PickingPass(RenderQueueCollection& rq);
    void DrawScreenQuad(GLuint textureToDraw);
    void DrawScene(RenderQueueCollection& rq);

    //----------------------Forward+-----------------------//
    void CalculateFrustum();
    void CullLights();
    //Frustum
    struct Plane {
        glm::vec3 Normal;
        float d;
    };
    struct Frustum {
        Plane Planes[4];
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
    GLuint m_FrustumSSBO = 0;
    GLuint m_LightSSBO = 1;
    GLuint m_LightGridSSBO = 2;
    GLuint m_LightOffsetSSBO = 3;
    GLuint m_LightIndexSSBO = 4;

    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type);
	//--------------------ShaderPrograms-------------------//
	ShaderProgram m_BasicForwardProgram;
    ShaderProgram m_DrawScreenQuadProgram;
    ShaderProgram m_CalculateFrustumProgram;
    ShaderProgram m_LightCullProgram;

};

#endif