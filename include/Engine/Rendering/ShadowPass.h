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


private:

   

    EventBroker* m_EventBroker;

    const IRenderer* m_Renderer;

    GLuint m_DepthMap;

    FrameBuffer m_DepthBuffer;

    ShaderProgram* m_ShadowProgram;

    GLuint m_DepthFBO;

    GLfloat m_NearPlane = -40.f;
    GLfloat m_FarPlane = 80.f;
    //GLfloat m_Left = -10.f;
    //GLfloat m_Right = 10.f;
    //GLfloat m_Bottom = -10.f;
    //GLfloat m_Top = 10.f;
    GLfloat m_LRBT[4] = { -40.f, 100.f, -50.f, 50.f };

};

#endif