#ifndef XboxControllerInputHandler_h__
#define XboxControllerInputHandler_h__

#include "InputHandler.h"
#include "Core/EKeyDown.h"
#include "Core/EKeyUp.h"

struct _XINPUT_STATE;
typedef _XINPUT_STATE XINPUT_STATE;

class XboxControllerInputHandler : public InputHandler
{
public:
    XboxControllerInputHandler(EventBroker* eventBroker, InputProxy* inputProxy);


    bool BindOrigin(std::string origin, std::string command, float value) override;

    void Update(double dt) override;

    virtual float GetCommandValue(std::string command) override;
private:
    std::unordered_map<std::string, int> m_OriginButtons;
    std::unordered_map<std::string, int> m_OriginAxes;
	std::unordered_map<int, std::tuple<std::string, float>> m_ButtonBindings; // GLFW_KEY... -> command string & value
	std::unordered_map<int, std::tuple<std::string, float>> m_AxisBindings; // GLFW_KEY... -> command string & value
    std::unordered_map<std::string, float> m_CommandValues;

	static const short MAX_GAMEPADS = 4;

    float getAxisValue(XINPUT_STATE& state, int axis);
	//typedef std::array<float, static_cast<int>(Gamepad::Axis::LAST) + 1> GamepadAxisState;
	//std::array<GamepadAxisState, MAX_GAMEPADS> m_CurrentGamepadAxisState;
	//std::array<GamepadAxisState, MAX_GAMEPADS> m_LastGamepadAxisState;
	//typedef std::array<bool, static_cast<int>(Gamepad::Button::LAST) + 1> GamepadButtonState;
	//std::array<GamepadButtonState, MAX_GAMEPADS> m_CurrentGamepadButtonState;
	//std::array<GamepadButtonState, MAX_GAMEPADS> m_LastGamepadButtonState;
};

#endif
