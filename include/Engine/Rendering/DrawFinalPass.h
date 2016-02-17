#ifndef DrawFinalPass_h__
#define DrawFinalPass_h__

#include "IRenderer.h"
#include "DrawFinalPassState.h"
#include "LightCullingPass.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Util/UnorderedMapVec2.h"
#include "Util/CommonFunctions.h"
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
    void ClearBuffer();

    //Return the texture that is used in later stages to apply the bloom effect
    GLuint BloomTexture() const { return m_BloomTexture; }
    GLuint BloomTextureLowRes() const { return m_BloomTextureLowRes; }
    //Return the texture with diffuse and lighting of the scene.
    GLuint SceneTexture() const { return m_SceneTexture; }
    GLuint SceneTextureLowRes() const { return m_SceneTextureLowRes; }
    //Return the framebuffer used in the scene rendering stage.
    FrameBuffer* FinalPassFrameBuffer() { return &m_FinalPassFrameBuffer; }
    FrameBuffer* FinalPassFrameBufferLowRes() { return &m_FinalPassFrameBufferLowRes; }


private:
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const;
    void GenerateMipMapTexture(GLuint* texture, GLenum wrapping, glm::vec2 dimensions, GLint format, GLenum type, GLint numMipMaps) const;

    void DrawSprites(std::list<std::shared_ptr<RenderJob>>&jobs, RenderScene& scene);
    void DrawModelRenderQueues(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);
    void DrawShieldToStencilBuffer(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);
    void DrawShieldedModelRenderQueue(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);
    void DrawToDepthBuffer(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);

    void BindExplosionUniforms(GLuint shaderHandle, std::shared_ptr<ExplosionEffectJob>& job, RenderScene& scene);
    void BindModelUniforms(GLuint shaderHandle, std::shared_ptr<ModelJob>& job, RenderScene& scene);

    void BindExplosionTextures(GLuint shaderHandle, std::shared_ptr<ExplosionEffectJob>& job);
    void BindModelTextures(GLuint shaderHandle, std::shared_ptr<ModelJob>& job);

    Texture* m_WhiteTexture;
    Texture* m_BlackTexture;
    Texture* m_NeutralNormalTexture;
    Texture* m_GreyTexture;
    Texture* m_ErrorTexture;

    FrameBuffer m_FinalPassFrameBuffer;
    FrameBuffer m_FinalPassFrameBufferLowRes;
    GLuint m_BloomTexture;
    GLuint m_SceneTexture;
    GLuint m_BloomTextureLowRes;
    GLuint m_SceneTextureLowRes;
    GLuint m_DepthBuffer;
    GLuint m_DepthBufferLowRes;

    //maqke this component based i guess?
    GLuint m_ShieldPixelRate = 16;

    const IRenderer* m_Renderer;
    const LightCullingPass* m_LightCullingPass;

    ShaderProgram* m_ForwardPlusProgram;
    ShaderProgram* m_ExplosionEffectProgram;
	ShaderProgram* m_ExplosionEffectSplatMapProgram;
    ShaderProgram* m_SpriteProgram;
	ShaderProgram* m_ForwardPlusSplatMapProgram;
    ShaderProgram* m_ShieldToStencilProgram;
    ShaderProgram* m_FillDepthBufferProgram;


	ShaderProgram* m_ForwardPlusSkinnedProgram;
	ShaderProgram* m_ExplosionEffectSkinnedProgram;
	ShaderProgram* m_ExplosionEffectSplatMapSkinnedProgram;
	ShaderProgram* m_ForwardPlusSplatMapSkinnedProgram;
    ShaderProgram* m_ShieldToStencilSkinnedProgram;
    ShaderProgram* m_FillDepthBufferSkinnedProgram;
};

#endif 