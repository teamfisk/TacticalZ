#include "Core/InputManager.h"

void InputManager::Initialize()
{
	// TODO: Gamepad
	//m_LastGamepadAxisState = std::array<GamepadAxisState, XUSER_MAX_COUNT>();
	//m_LastGamepadButtonState = std::array<GamepadButtonState, XUSER_MAX_COUNT>();

	EVENT_SUBSCRIBE_MEMBER(m_ELockMouse, &InputManager::OnLockMouse);
	EVENT_SUBSCRIBE_MEMBER(m_EUnlockMouse, &InputManager::OnUnlockMouse);

    //glfwSetMouseButtonCallback(m_GLFWWindow, &InputManager::GLFWMouseButtonCallback);
}

void InputManager::Update(double dt)
{
	m_EventBroker->Process<InputManager>();

	m_LastKeyState = m_CurrentKeyState;
	m_LastMouseState = m_CurrentMouseState;
	m_LastMouseX = m_CurrentMouseX;
	m_LastMouseY = m_CurrentMouseY;

	// Keyboard input
	for (int i = 0; i <= GLFW_KEY_LAST; ++i) {
		m_CurrentKeyState[i] = glfwGetKey(m_GLFWWindow, i);
		if (m_CurrentKeyState[i] != m_LastKeyState[i]) {
			// Publish key events
			if (m_CurrentKeyState[i]) {
				Events::KeyDown e;
				e.KeyCode = i;
				m_EventBroker->Publish(e);
			} else {
				Events::KeyUp e;
				e.KeyCode = i;
				m_EventBroker->Publish(e);
			}
		}
	}

	// Mouse buttons
	for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
		m_CurrentMouseState[i] = glfwGetMouseButton(m_GLFWWindow, i);
		if (m_CurrentMouseState[i] != m_LastMouseState[i]) {
			double x, y;
			glfwGetCursorPos(m_GLFWWindow, &x, &y);
			// Publish mouse button events
			if (m_CurrentMouseState[i]) {
				Events::MousePress e;
				e.Button = i;
				e.X = x;
				e.Y = y;
				m_EventBroker->Publish(e);
			} else {
				Events::MouseRelease e;
				e.Button = i;
				e.X = x;
				e.Y = y;
				m_EventBroker->Publish(e);
			}
		}
	}

	// Mouse movement
	glfwGetCursorPos(m_GLFWWindow, &m_CurrentMouseX, &m_CurrentMouseY);
	m_CurrentMouseDeltaX = m_CurrentMouseX - m_LastMouseX;
	m_CurrentMouseDeltaY = m_CurrentMouseY - m_LastMouseY;
	if (m_CurrentMouseDeltaX != 0 || m_CurrentMouseDeltaY != 0) {
		// Publish mouse move events
		Events::MouseMove e;
		e.X = m_CurrentMouseX;
		e.Y = m_CurrentMouseY;
		e.DeltaX = m_CurrentMouseDeltaX;
		e.DeltaY = m_CurrentMouseDeltaY;
		m_EventBroker->Publish(e);
	}

    // Joysticks
    //for (int i = 0; i < GLFW_JOYSTICK_LAST; i++) {
    //    if (glfwJoystickPresent(i) == GL_FALSE) {
    //        continue;
    //    }

    //    int count;
    //    const float* axes = glfwGetJoystickAxes(i, &count);
    //    LOG_DEBUG("Controller %i NumAxes: %i", i, count);
    //    m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::LeftX)] = axes[0];
    //    m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::LeftY)] = axes[1];
    //    m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::RightX)] = axes[2];
    //    m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::RightY)] = axes[3];
    //}

	// // Lock mouse while holding LMB
	// if (m_CurrentMouseState[GLFW_MOUSE_BUTTON_LEFT])
	// {
	// 	m_LastMouseX = m_Renderer->Width() / 2.f; // xpos;
	// 	m_LastMouseY = m_Renderer->Height() / 2.f; // ypos;
	// 	glfwSetCursorPos(m_GLFWWindow, m_LastMouseX, m_LastMouseY);
	// }
	// // Hide/show cursor with LMB
	// if (m_CurrentMouseState[GLFW_MOUSE_BUTTON_LEFT] && !m_LastMouseState[GLFW_MOUSE_BUTTON_LEFT])
	// {
	// 	glfwSetInputMode(m_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	// }
	// if (!m_CurrentMouseState[GLFW_MOUSE_BUTTON_LEFT] && m_LastMouseState[GLFW_MOUSE_BUTTON_LEFT])
	// {
	// 	glfwSetInputMode(m_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	// }

	// TODO: Xbox360 controller
	/**/

	m_LastKeyState = m_CurrentKeyState;
	m_LastMouseState = m_CurrentMouseState;
	m_LastMouseX = m_CurrentMouseX;
	m_LastMouseY = m_CurrentMouseY;
	//m_LastGamepadAxisState = m_CurrentGamepadAxisState;
	//m_LastGamepadButtonState = m_CurrentGamepadButtonState;
}

void InputManager::PublishGamepadAxisIfChanged(int gamepadID, Gamepad::Axis axis)
{
	//float currentValue = m_CurrentGamepadAxisState[gamepadID][static_cast<int>(axis)];
	//float lastValue = m_LastGamepadAxisState[gamepadID][static_cast<int>(axis)];
	//if (currentValue != lastValue) {
	//	Events::GamepadAxis e;
	//	e.GamepadID = gamepadID;
	//	e.Axis = axis;
	//	e.Value = currentValue;
	//	m_EventBroker->Publish(e);
	//}
}

void InputManager::PublishGamepadButtonIfChanged(int gamepadID, Gamepad::Button button)
{
	//bool currentState = m_CurrentGamepadButtonState[gamepadID][static_cast<int>(button)];
	//float lastState = m_LastGamepadButtonState[gamepadID][static_cast<int>(button)];
	//if (currentState != lastState) {
	//	if (currentState == true) {
	//		Events::GamepadButtonDown e;
	//		e.GamepadID = gamepadID;
	//		e.Button = button;
	//		m_EventBroker->Publish(e);
	//	} else {
	//		Events::GamepadButtonUp e;
	//		e.GamepadID = gamepadID;
	//		e.Button = button;
	//		m_EventBroker->Publish(e);
	//	}
	//}
}

bool InputManager::OnLockMouse(const Events::LockMouse &event)
{
	m_MouseLocked = true;
	glfwSetInputMode(m_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

bool InputManager::OnUnlockMouse(const Events::UnlockMouse &event)
{
	m_MouseLocked = false;
	glfwSetInputMode(m_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	return true;
}
