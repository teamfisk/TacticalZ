#ifndef MouseInputHandler_h__
#define MouseInputHandler_h__

#include <GLFW/glfw3.h>
#include "InputHandler.h"
#include "Core/EMousePress.h"
#include "Core/EMouseRelease.h"
#include "Core/EMouseMove.h"

class MouseInputHandler : public InputHandler
{
public:
    MouseInputHandler(EventBroker* eventBroker, InputProxy* inputProxy);

    bool BindOrigin(std::string origin, std::string command, float value) override;

private:
    std::unordered_map<std::string, int> m_OriginCodes;
    std::unordered_map<std::string, char> m_OriginAxes;
	std::unordered_map<int, std::tuple<std::string, float>> m_Bindings; // GLFW_MOUSE_BUTTON... -> command string & value
	std::unordered_map<char, std::tuple<std::string, float>> m_Axes; // Axis -> command string & value

	EventRelay<InputHandler, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
	EventRelay<InputHandler, Events::MouseRelease> m_EMouseRelease;
	bool OnMouseRelease(const Events::MouseRelease& e);
	EventRelay<InputHandler, Events::MouseMove> m_EMouseMove;
	bool OnMouseMove(const Events::MouseMove& e);

    bool hasOrigin(std::string origin);
};

#endif
