#ifndef InputSystem_h__
#define InputSystem_h__

#include <array>
#include <unordered_map>

#include "Core/System.h"
#include "Core/EKeyUp.h"
#include "Core/EKeyDown.h"
#include "Core/EMousePress.h"
#include "Core/EMouseRelease.h"
#include "Core/EGamepadAxis.h"
#include "Core/EGamepadButton.h"
#include "Core/Util/EnumClassHash.h"
#include "EBindKey.h"
#include "EBindMouseButton.h"
#include "EBindGamepadAxis.h"
#include "EBindGamepadButton.h"
#include "EInputCommand.h"

namespace Systems
{

class InputSystem : public System
{
public:
	InputSystem(World* world, std::shared_ptr<dd::EventBroker> eventBroker)
		: System(world, eventBroker)
	{ }

	void RegisterComponents(ComponentFactory* cf) override;
	void Initialize() override;

	void Update(double dt) override;

private:
	std::unordered_map<std::string, std::unordered_map<int, float>> m_CommandKeyboardValues; // command string -> keyboard key value for command
	std::unordered_map<std::string, std::unordered_map<int, float>> m_CommandMouseButtonValues; // command string -> mouse button value for command
	std::unordered_map<std::string, std::unordered_map<Gamepad::Axis, float, EnumClassHash>> m_CommandGamepadAxisValues; // command string -> gamepad axis value for command
	std::unordered_map<std::string, std::unordered_map<Gamepad::Button, float, EnumClassHash>> m_CommandGamepadButtonValues; // command string -> gamepad button value for command
	// Input binding tables
	std::unordered_multimap<int, std::tuple<std::string, float>> m_KeyBindings; // GLFW_KEY... -> command string & value
	std::unordered_multimap<int, std::tuple<std::string, float>> m_MouseButtonBindings; // GLFW_MOUSE_BUTTON... -> command string
	std::unordered_multimap<Gamepad::Axis, std::tuple<std::string, float>, EnumClassHash> m_GamepadAxisBindings; // Gamepad::Axis -> command string & value
	std::unordered_multimap<Gamepad::Button, std::tuple<std::string, float>, EnumClassHash> m_GamepadButtonBindings; // Gamepad::Button -> command string

	// Input events
	EventRelay<InputSystem, Events::KeyDown> m_EKeyDown;
	bool OnKeyDown(const Events::KeyDown &event);
	EventRelay<InputSystem, Events::KeyUp> m_EKeyUp;
	bool OnKeyUp(const Events::KeyUp &event);
	EventRelay<InputSystem, Events::MousePress> m_EMousePress;
	bool OnMousePress(const Events::MousePress &event);
	EventRelay<InputSystem, Events::MouseRelease> m_EMouseRelease;
	bool OnMouseRelease(const Events::MouseRelease &event);
	EventRelay<InputSystem, Events::GamepadAxis> m_EGamepadAxis;
	bool OnGamepadAxis(const Events::GamepadAxis &event);
	EventRelay<InputSystem, Events::GamepadButtonDown> m_EGamepadButtonDown;
	bool OnGamepadButtonDown(const Events::GamepadButtonDown &event);
	EventRelay<InputSystem, Events::GamepadButtonUp> m_EGamepadButtonUp;
	bool OnGamepadButtonUp(const Events::GamepadButtonUp &event);
	// Input binding events
	EventRelay<InputSystem, Events::BindKey> m_EBindKey;
	bool OnBindKey(const Events::BindKey &event);
	EventRelay<InputSystem, Events::BindMouseButton> m_EBindMouseButton;
	bool OnBindMouseButton(const Events::BindMouseButton &event);
	EventRelay<InputSystem, Events::BindGamepadAxis> m_EBindGamepadAxis;
	bool OnBindGamepadAxis(const Events::BindGamepadAxis &event);
	EventRelay<InputSystem, Events::BindGamepadButton> m_EBindGamepadButton;
	bool OnBindGamepadButton(const Events::BindGamepadButton &event);

	float GetCommandTotalValue(std::string command);
	void PublishCommand(int playerID, std::string command, float value);
};

}

#endif
