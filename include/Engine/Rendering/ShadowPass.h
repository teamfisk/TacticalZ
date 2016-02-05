#ifndef ShadowPass_h_
#define ShadowPass_h_

#include "IRenderer.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "../Core/EventBroker.h"
#include "../Core/World.h"
#include "ShadowPassState.h"

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

};

#endif