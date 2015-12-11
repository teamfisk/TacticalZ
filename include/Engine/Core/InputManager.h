#ifndef InputManager_h__
#define InputManager_h__

#include <array>
#include <GLFW/glfw3.h>

#include "../Common.h"
#include "EventBroker.h"
#include "EKeyDown.h"
#include "EKeyUp.h"
#include "EMousePress.h"
#include "EMouseRelease.h"
#include "EMouseMove.h"
#include "ELockMouse.h"
#include "EGamepadAxis.h"
#include "EGamepadButton.h"

class InputManager
{
public:
	InputManager(GLFWwindow* window, EventBroker* eventBroker)
		: m_GLFWWindow(window)
		, m_EventBroker(eventBroker)
		, m_CurrentKeyState()
		, m_LastKeyState()
		, m_CurrentMouseState()
		, m_LastMouseState()
		, m_CurrentMouseX(0), m_CurrentMouseY(0)
		, m_LastMouseX(0), m_LastMouseY(0)
		, m_CurrentMouseDeltaX(0), m_CurrentMouseDeltaY(0)
		, m_MouseLocked(false)
	{ Initialize(); }

	void Initialize();


	void Update(double dt);

private:
	GLFWwindow* m_GLFWWindow;
	EventBroker* m_EventBroker;

	EventRelay<InputManager, Events::LockMouse> m_ELockMouse;
	bool OnLockMouse(const Events::LockMouse &event);
	EventRelay<InputManager, Events::UnlockMouse> m_EUnlockMouse;
	bool OnUnlockMouse(const Events::UnlockMouse &event);

	std::array<int, GLFW_KEY_LAST+1> m_CurrentKeyState;
	std::array<int, GLFW_KEY_LAST+1> m_LastKeyState;
	std::array<int, GLFW_MOUSE_BUTTON_LAST+1> m_CurrentMouseState;
	std::array<int, GLFW_MOUSE_BUTTON_LAST+1> m_LastMouseState;

	double m_CurrentMouseX, m_CurrentMouseY;
	double m_LastMouseX, m_LastMouseY;
	double m_CurrentMouseDeltaX, m_CurrentMouseDeltaY;
	bool m_MouseLocked;

	void PublishGamepadAxisIfChanged(int gamepadID, Gamepad::Axis axis);
	void PublishGamepadButtonIfChanged(int gamepadID, Gamepad::Button button);

    static void GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        LOG_DEBUG("Click! %i %i %i", button, action, mods);
    }
};

#endif
