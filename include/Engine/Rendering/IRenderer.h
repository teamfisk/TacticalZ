#ifndef IRenderer_h__
#define IRenderer_h__

#include "../Common.h"
#include "../OpenGL.h"
#include "../Core/Util/Rectangle.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "Model.h"

class IRenderer
{
public:
	GLFWwindow* Window() const { return m_Window; }
	Rectangle Resolution() const { return m_Resolution; }
	void SetResolution(const Rectangle& resolution) { m_Resolution = resolution; }
	bool Fullscreen() { return m_Fullscreen; }
	void SetFullscreen(bool fullscreen) { m_Fullscreen = fullscreen; }
	bool VSYNC() const { return m_VSYNC; }
	void SetVSYNC(bool vsync) { m_VSYNC = vsync; }
	::Camera* Camera() const { return m_Camera; }
	void SetCamera(::Camera* camera)
	{
		if (camera == nullptr) {
			m_Camera = m_DefaultCamera;
		} else {
			m_Camera = camera;
		}
	}

	virtual void Initialize() = 0;
	virtual void Draw(RenderQueueCollection& rq) = 0;

protected:
	Rectangle m_Resolution = Rectangle(1280, 720);
	bool m_Fullscreen = false;
	bool m_VSYNC = false;
	int m_GLVersion[2];
	std::string m_GLVendor;
	::Camera* m_DefaultCamera;
	::Camera* m_Camera = nullptr;
	GLFWwindow* m_Window = nullptr;
};

#endif // Renderer_h__
