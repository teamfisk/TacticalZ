#ifndef DrawFinalPass_h__
#define DrawFinalPass_h__

#include "IRenderer.h"
#include "DrawFinalPassState.h"
#include "LightCullingPass.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Util/UnorderedMapVec2.h"
#include "Texture.h"

class DrawFinalPass
{
public:
    DrawFinalPass(IRenderer* renderer, LightCullingPass* lightCullingPass);
    ~DrawFinalPass() { }
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();

    void Draw(RenderScene& scene);

    //Getters


private:
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const;

    Texture* m_WhiteTexture;

    const IRenderer* m_Renderer;
    const LightCullingPass* m_LightCullingPass;

    ShaderProgram* m_ForwardPlusProgram;
};

#endif 