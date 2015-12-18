#ifndef LightCullingPass_h__
#define LightCullingPass_h__

#include "IRenderer.h"
#include "LightCullingPassState.h"
#include "ShaderProgram.h"


class LightCullingPass
{
public:
    LightCullingPass();
    ~LightCullingPass();


    void GenerateNewFrustum();
private:
    void CullLights();

    void InitializeTextures();
    void InitializeSSBOs();
    void InitializeShaderPrograms();


};


#endif