#ifndef Renderer_h__
#define Renderer_h__

#include <sstream>

#include "IRenderer.h"
#include "ShaderProgram.h"
//TODO: Temp resourceManager
#include "../Core/ResourceManager.h"
#include "Util/UnorderedMapVec2.h"
#include "Util/FrameBuffer.h"
#include "../Core/World.h"

class Renderer : public IRenderer
{
public:
	virtual void Initialize() override;
	virtual void Update(double dt) override;
	virtual void Draw(RenderQueueCollection& rq) override;

private:
	//----------------------Variables----------------------//
	Texture* m_ErrorTexture;
	Texture* m_WhiteTexture;
	float m_CameraMoveSpeed;
    FrameBuffer m_PickingBuffer;
    GLuint m_PickingTexture;
    GLuint m_DepthBuffer;

    Model* m_ScreenQuad;
    Model* m_UnitQuad;
    Model* m_UnitSphere;


    
    std::unordered_map<glm::vec2, EntityID> m_PickingColorsToEntity;

	//----------------------Functions----------------------//
	void InitializeWindow();
	void InitializeShaders();
    void InitializeTextures();
    void InitializeFrameBuffers();
    //TODO: Renderer: Get InputUpdate out of renderer
	void InputUpdate(double dt);
    void PickingPass(RenderQueueCollection& rq);
    void DrawScreenQuad(GLuint textureToDraw);
    void DrawScene(RenderQueueCollection& rq);


    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type);
	//--------------------ShaderPrograms-------------------//
	ShaderProgram m_BasicForwardProgram;
    ShaderProgram m_PickingProgram;
    ShaderProgram m_DrawScreenQuadProgram;
};

#endif