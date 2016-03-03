#ifndef DrawFinalPass_h__
#define DrawFinalPass_h__

#include "IRenderer.h"
#include "DrawFinalPassState.h"
#include "LightCullingPass.h"
#include "CubeMapPass.h"
#include "SSAOPass.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Util/UnorderedMapVec2.h"
#include "Util/CommonFunctions.h"
#include "Texture.h"
#include "ShadowPass.h"

class DrawFinalPass
{
public:
    DrawFinalPass(IRenderer* renderer, LightCullingPass* lightCullingPass, CubeMapPass* cubeMapPass, SSAOPass* ssaoPass, ShadowPass* shadowPass);
    ~DrawFinalPass() { }
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();
    void Draw(RenderScene& scene);
    void ClearBuffer();
    void OnWindowResize();

    //Return the texture that is used in later stages to apply the bloom effect
    GLuint BloomTexture() const { return m_BloomTexture; }
    //Return the texture with diffuse and lighting of the scene.
    GLuint SceneTexture() const { return m_SceneTexture; }
    //Return the framebuffer used in the scene rendering stage.
    FrameBuffer* FinalPassFrameBuffer() { return &m_FinalPassFrameBuffer; }

private:
    void DrawSprites(std::list<std::shared_ptr<RenderJob>>&jobs, RenderScene& scene);
    void DrawModelRenderQueues(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);
	void DrawModelRenderQueuesWithShieldCheck(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);
    void DrawShieldedModelRenderQueue(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);
    void DrawToDepthStencilBuffer(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);

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
	FrameBuffer m_ShieldDepthFrameBuffer;
    GLuint m_BloomTexture;
    GLuint m_SceneTexture;
    GLuint m_DepthBuffer;
	GLuint m_ShieldBuffer;
    GLuint m_CubeMapTexture;

    //maqke this component based i guess?
    GLuint m_ShieldPixelRate = 16;

    const IRenderer* m_Renderer;
    const LightCullingPass* m_LightCullingPass;
    const ShadowPass* m_ShadowPass;
    const CubeMapPass* m_CubeMapPass;
	const SSAOPass* m_SSAOPass;

    ShaderProgram* m_ForwardPlusProgram;
    ShaderProgram* m_ExplosionEffectProgram;
	ShaderProgram* m_ExplosionEffectSplatMapProgram;
    ShaderProgram* m_SpriteProgram;
	ShaderProgram* m_ForwardPlusSplatMapProgram;
    ShaderProgram* m_FillDepthStencilBufferProgram;

	ShaderProgram* m_ForwardPlusShieldCheckProgram;
	ShaderProgram* m_ExplosionEffectShieldCheckProgram;
	ShaderProgram* m_ExplosionEffectSplatMapShieldCheckProgram;
	ShaderProgram* m_SpriteShieldCheckProgram;
	ShaderProgram* m_ForwardPlusSplatMapShieldCheckProgram;



	ShaderProgram* m_ForwardPlusSkinnedProgram;
	ShaderProgram* m_ExplosionEffectSkinnedProgram;
	ShaderProgram* m_ExplosionEffectSplatMapSkinnedProgram;
	ShaderProgram* m_ForwardPlusSplatMapSkinnedProgram;
    ShaderProgram* m_FillDepthStencilBufferSkinnedProgram;

	ShaderProgram* m_ForwardPlusSkinnedShieldCheckProgram;
	ShaderProgram* m_ExplosionEffectSkinnedShieldCheckProgram;
	ShaderProgram* m_ExplosionEffectSplatMapSkinnedShieldCheckProgram;
	ShaderProgram* m_ForwardPlusSplatMapSkinnedShieldCheckProgram;
	ShaderProgram* m_FillDepthBufferSkinnedShieldCheckProgram;
};

#endif 