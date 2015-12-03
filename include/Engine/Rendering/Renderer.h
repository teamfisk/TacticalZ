#ifndef Renderer_h__
#define Renderer_h__

#include <sstream>

#include "IRenderer.h"
#include "ShaderProgram.h"
//TODO: Temp resourceManager
#include "../Core/ResourceManager.h"
#include "Util/UnorderedMapVec2.h"

class Renderer : public IRenderer
{
public:
	virtual void Initialize() override;
	virtual void Update(double dt) override;
	virtual void Draw(RenderQueueCollection& rq) override;

private:
	//----------------------Variables----------------------//
	RenderQueueCollection m_TempRQ;
	Texture* m_ErrorTexture;
	Texture* m_WhiteTexture;
	float m_CameraMoveSpeed;
    GLuint m_PickingBuffer;
    GLuint m_PickingTexture;
    GLuint m_DepthBuffer;

    Model* m_ScreenQuad;
    Model* m_UnitQuad;
    Model* m_UnitSphere;

    //Temporary
    Model* m;
    Model* m2;
    Model* m3;
    Model* MapModel;

    
    std::unordered_map<glm::vec2, const Model*> m_PickingColorsToModels;

	//----------------------Functions----------------------//
	void InitializeWindow();
	void InitializeShaders();
    void InitializeTextures();
    void InitializeFrameBuffers();
	//TODO: Render: Remove ModelsToDraw from Renderer.
	void ModelsToDraw();
	void EnqueueModel(Model* model);
	void InputUpdate(double dt);
    void PickingPass();
    void DrawScreenQuad(GLuint textureToDraw);
    glm::vec3 GetClickedPixelData(float x, float y);
    void DrawScene(RenderQueueCollection& rq);

    //--------------------Utility functions----------------//
    glm::vec3 ScreenCoordsToWorldPos(glm::vec2 screenCoord, float depth);
    //EntityID ScreenCoordsToEntityID(glm::vec2 screenCoord, float depth);

	//--------------------ShaderPrograms-------------------//
	ShaderProgram m_BasicForwardProgram;
    ShaderProgram m_PickingProgram;
    ShaderProgram m_DrawScreenQuadProgram;
};

#endif