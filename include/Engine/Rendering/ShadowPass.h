#ifndef ShadowPass_h_
#define ShadowPass_h_

#include "IRenderer.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "../Core/EventBroker.h"
#include "../Core/World.h"
#include "ShadowPassState.h"
#include "imgui/imgui.h"

//#include "ShadowPassState.h" // not created yet

enum NearFar { Near = 0, Far = 1 };
enum LRBT { Left = 0, Right = 1, Bottom = 2, Top = 3 };

class ShadowPass
{
public:
    
    ShadowPass(IRenderer* renderer);
    ~ShadowPass();
    
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();
    void ClearBuffer();
    void Draw(RenderScene& scene);

 

    GLuint DepthMap() const { return m_DepthMap; }
    glm::mat4 lightSpaceMatrix() const { return m_LightSpaceMatrix; }
    glm::mat4 lightP() const { return m_LightProjection; }
    glm::mat4 lightV() const { return m_LightView; }
    //glm::mat4 lightV() const { return m_LightProjection; } //swapped m_P -> m_V
    //glm::mat4 lightP() const { return m_LightView; } // swapped m_V -> m_P

private:

   

    EventBroker* m_EventBroker;

    const IRenderer* m_Renderer;

    GLuint m_DepthMap;

    FrameBuffer m_DepthBuffer;

    ShaderProgram* m_ShadowProgram;

    GLuint m_DepthFBO;

    GLfloat m_NearFarPlane[2] = { -40.f, 30.f };
    GLfloat m_LRBT[4] = { -40.f, 100.f, -50.f, 50.f };

    glm::mat4 m_LightProjection;
    glm::mat4 m_LightView;
    glm::mat4 m_LightSpaceMatrix;
};

#endif