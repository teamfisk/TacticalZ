#ifndef KeyboardInputHandler_h__
#define KeyboardInputHandler_h__

#include <GLFW/glfw3.h>
#include "InputHandler.h"
#include "Core/EKeyDown.h"
#include "Core/EKeyUp.h"

class KeyboardInputHandler : public InputHandler
{
public:
    KeyboardInputHandler(EventBroker* eventBroker, InputProxy* inputProxy);

    bool BindOrigin(std::string origin, std::string command, float value) override;

private:
    std::unordered_map<std::string, int> m_OriginKeyCodes;
	std::unordered_map<int, std::tuple<std::string, float>> m_KeyBindings; // GLFW_KEY... -> command string & value

	EventRelay<InputHandler, Events::KeyDown> m_EKeyDown;
    bool OnKeyDown(const Events::KeyDown& e);
	EventRelay<InputHandler, Events::KeyUp> m_EKeyUp;
	bool OnKeyUp(const Events::KeyUp& e);
};

#endif