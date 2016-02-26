#ifndef IRenderer_h__
#define IRenderer_h__

#include "../Common.h"
#include "../OpenGL.h"
#include "../GLM.h"
#include "../Core/Util/Rectangle.h"
#include "../Core/ConfigFile.h"
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
    //Returns screen size including window border and header
	Rectangle Resolution() const { return m_Resolution; }
	virtual void SetResolution(const Rectangle& resolution) { m_Resolution = resolution; }
	bool Fullscreen() { return m_Fullscreen; }
	virtual void SetFullscreen(bool fullscreen) { m_Fullscreen = fullscreen; }
	bool VSYNC() const { return m_VSYNC; }
	virtual void SetVSYNC(bool vsync) { m_VSYNC = vsync; }
    std::string WindowTitle() const { return m_WindowTitle; }
    virtual void SetWindowTitle(const std::string& title) { glfwSetWindowTitle(m_Window, title.c_str()); m_WindowTitle = title; }
    //Returns screen size excluding window border and header
    Rectangle GetViewportSize() const { return m_ViewportSize; }
	virtual void Initialize() = 0;
	virtual void Update(double dt) = 0;
	virtual void Draw(RenderFrame& rq) = 0;
    virtual PickData Pick(glm::vec2 screenCord) = 0;

protected:
	Rectangle m_Resolution = Rectangle::Rectangle(1280, 720);
    Rectangle m_ViewportSize = Rectangle::Rectangle(1280, 720);
	bool m_Fullscreen = false;
	bool m_VSYNC = false;
	int m_GLVersion[2];
	std::string m_GLVendor;
	GLFWwindow* m_Window = nullptr;
    std::string m_WindowTitle;
};

#endif // Renderer_h__
