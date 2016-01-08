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

    void Draw(RenderQueueCollection& rq);

    //Getters


private:
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const;

    Texture* m_WhiteTexture;

    const IRenderer* m_Renderer;

    ShaderProgram* m_BasicForwardProgram;

    ShaderProgram* m_CoolDeathAnimProgram;

    glm::vec3 Origin;
    GLfloat TimeDeath = 0.f;
    GLfloat EndDeath = 0.f;

};

#endif 