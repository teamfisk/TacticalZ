#include "Core/InputManager.h"

void InputManager::Initialize()
{
	// TODO: Gamepad
	//m_LastGamepadAxisState = std::array<GamepadAxisState, XUSER_MAX_COUNT>();
	//m_LastGamepadButtonState = std::array<GamepadButtonState, XUSER_MAX_COUNT>();

	EVENT_SUBSCRIBE_MEMBER(m_ELockMouse, &InputManager::OnLockMouse);
	EVENT_SUBSCRIBE_MEMBER(m_EUnlockMouse, &InputManager::OnUnlockMouse);
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
	/*DWORD dwResult;
	for (int i = 0; i < MAX_GAMEPADS; i++)
	{
		XINPUT_STATE state = { 0 };
		// Simply get the state of the controller from XInput.
		dwResult = XInputGetState(i, &state);
		if (dwResult == 0)
		{
			if(std::abs(state.Gamepad.sThumbLX) <= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				state.Gamepad.sThumbLX = 0;
			if(std::abs(state.Gamepad.sThumbLY) <= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				state.Gamepad.sThumbLY = 0;
			if(std::abs(state.Gamepad.sThumbRX) <= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
				state.Gamepad.sThumbRX = 0;
			if(std::abs(state.Gamepad.sThumbRY) <= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
				state.Gamepad.sThumbRY = 0;
			if(std::abs(state.Gamepad.bLeftTrigger) <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				state.Gamepad.bLeftTrigger = 0;
			if(std::abs(state.Gamepad.bRightTrigger) <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				state.Gamepad.bRightTrigger = 0;

			m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::LeftX)] = state.Gamepad.sThumbLX / 32767.f;
			m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::LeftY)] = state.Gamepad.sThumbLY / 32767.f;
			m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::RightX)] = state.Gamepad.sThumbRX / 32767.f;
			m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::RightY)] = state.Gamepad.sThumbRY / 32767.f;
			m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::LeftTrigger)] = state.Gamepad.bLeftTrigger / 255.f;
			m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::RightTrigger)] = state.Gamepad.bRightTrigger / 255.f;
			PublishGamepadAxisIfChanged(i, Gamepad::Axis::LeftX);
			PublishGamepadAxisIfChanged(i, Gamepad::Axis::LeftY);
			PublishGamepadAxisIfChanged(i, Gamepad::Axis::RightX);
			PublishGamepadAxisIfChanged(i, Gamepad::Axis::RightY);
			PublishGamepadAxisIfChanged(i, Gamepad::Axis::LeftTrigger);
			PublishGamepadAxisIfChanged(i, Gamepad::Axis::RightTrigger);

			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Up)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Down)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Left)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Right)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Start)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Back)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::LeftThumb)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::RightThumb)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::LeftShoulder)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::RightShoulder)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::A)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::B)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::X)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
			m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Y)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::Up);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::Down);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::Left);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::Right);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::Start);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::Back);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::LeftThumb);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::RightThumb);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::LeftShoulder);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::RightShoulder);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::A);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::B);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::X);
			PublishGamepadButtonIfChanged(i, Gamepad::Button::Y);
		}
	}*/

	m_LastKeyState = m_CurrentKeyState;
	m_LastMouseState = m_CurrentMouseState;
	m_LastMouseX = m_CurrentMouseX;
	m_LastMouseY = m_CurrentMouseY;
	m_LastGamepadAxisState = m_CurrentGamepadAxisState;
	m_LastGamepadButtonState = m_CurrentGamepadButtonState;
}

void InputManager::PublishGamepadAxisIfChanged(int gamepadID, Gamepad::Axis axis)
{
	float currentValue = m_CurrentGamepadAxisState[gamepadID][static_cast<int>(axis)];
	float lastValue = m_LastGamepadAxisState[gamepadID][static_cast<int>(axis)];
	if (currentValue != lastValue) {
		Events::GamepadAxis e;
		e.GamepadID = gamepadID;
		e.Axis = axis;
		e.Value = currentValue;
		m_EventBroker->Publish(e);
	}
}

void InputManager::PublishGamepadButtonIfChanged(int gamepadID, Gamepad::Button button)
{
	bool currentState = m_CurrentGamepadButtonState[gamepadID][static_cast<int>(button)];
	float lastState = m_LastGamepadButtonState[gamepadID][static_cast<int>(button)];
	if (currentState != lastState) {
		if (currentState == true) {
			Events::GamepadButtonDown e;
			e.GamepadID = gamepadID;
			e.Button = button;
			m_EventBroker->Publish(e);
		} else {
			Events::GamepadButtonUp e;
			e.GamepadID = gamepadID;
			e.Button = button;
			m_EventBroker->Publish(e);
		}
	}
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
