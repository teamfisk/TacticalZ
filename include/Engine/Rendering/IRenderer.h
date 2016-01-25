#ifndef IRenderer_h__
#define IRenderer_h__

#include "../Common.h"
#include "../OpenGL.h"
#include "../GLM.h"
#include "../Core/Util/Rectangle.h"
#include "Util/ScreenCoords.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "Model.h"
#include "../Core/World.h" //So temp


struct PickData
{
    EntityID Entity;
    glm::vec3 Position; //World position
    float Depth;
    ::Camera* Camera;
    const ::World* World;
};

class IRenderer
{
public:
	GLFWwindow* Window() const { return m_Window; }
    //Returns screensize including window border and header
	Rectangle Resolution() const { return m_Resolution; }
	void SetResolution(const Rectangle& resolution) { m_Resolution = resolution; }
	bool Fullscreen() { return m_Fullscreen; }
	void SetFullscreen(bool fullscreen) { m_Fullscreen = fullscreen; }
	bool VSYNC() const { return m_VSYNC; }
	void SetVSYNC(bool vsync) { m_VSYNC = vsync; }
    //Returns screensize excluding window border and header
    Rectangle GetViewPortSize() const { return m_ViewPortWidth; }
    void SetViewPortSize(const Rectangle& viewportWidth) { m_ViewPortWidth = viewportWidth; }
	virtual void Initialize() = 0;
	virtual void Update(double dt) = 0;
	virtual void Draw(RenderFrame& rq) = 0;
    virtual PickData Pick(glm::vec2 screenCord) = 0;


    World* m_World; //Temp world, untill viktor merge.

protected:
	Rectangle m_Resolution = Rectangle::Rectangle(1280, 720);
    Rectangle m_ViewPortWidth = Rectangle::Rectangle(1280, 720);
	bool m_Fullscreen = false;
	bool m_VSYNC = false;
	int m_GLVersion[2];
	std::string m_GLVendor;
	GLFWwindow* m_Window = nullptr;
};

#endif // Renderer_h__
