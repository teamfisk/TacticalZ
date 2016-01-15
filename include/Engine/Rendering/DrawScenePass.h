#ifndef DrawScenePass_h__
#define DrawScenePass_h__

#include "IRenderer.h"
#include "DrawScenePassState.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Util/UnorderedMapVec2.h"
#include "Texture.h"

class DrawScenePass
{
public:
    DrawScenePass(IRenderer* renderer);
    ~DrawScenePass() { }
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();

    void Draw(RenderScene& scene);

    //Getters


private:
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const;

    static bool DepthSort(const std::shared_ptr<RenderJob> &i, const std::shared_ptr<RenderJob> &j)
    {
        return (i->Depth < j->Depth);
    };

    Texture* m_WhiteTexture;

    const IRenderer* m_Renderer;

    ShaderProgram* m_BasicForwardProgram;


};

#endif 