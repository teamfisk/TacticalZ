#include "PrecompiledHeader.h"
#include "Input/InputSystem.h"
#include "Core/World.h"

void Systems::InputSystem::RegisterComponents(ComponentFactory* cf)
{
	
}

void Systems::InputSystem::Initialize()
{
	// Subscribe to events
	EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &Systems::InputSystem::OnKeyDown);
	EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &Systems::InputSystem::OnKeyUp);
	EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &Systems::InputSystem::OnMousePress);
	EVENT_SUBSCRIBE_MEMBER(m_EMouseRelease, &Systems::InputSystem::OnMouseRelease);
	EVENT_SUBSCRIBE_MEMBER(m_EGamepadAxis, &Systems::InputSystem::OnGamepadAxis);
	EVENT_SUBSCRIBE_MEMBER(m_EGamepadButtonDown, &Systems::InputSystem::OnGamepadButtonDown);
	EVENT_SUBSCRIBE_MEMBER(m_EGamepadButtonUp, &Systems::InputSystem::OnGamepadButtonUp);
	EVENT_SUBSCRIBE_MEMBER(m_EBindKey, &Systems::InputSystem::OnBindKey);
	EVENT_SUBSCRIBE_MEMBER(m_EBindMouseButton, &Systems::InputSystem::OnBindMouseButton);
	EVENT_SUBSCRIBE_MEMBER(m_EBindGamepadAxis, &Systems::InputSystem::OnBindGamepadAxis);
	EVENT_SUBSCRIBE_MEMBER(m_EBindGamepadButton, &Systems::InputSystem::OnBindGamepadButton);
}

void Systems::InputSystem::Update(double dt)
{

}

bool Systems::InputSystem::OnKeyDown(const Events::KeyDown &event)
{
	auto range = m_KeyBindings.equal_range(event.KeyCode);
	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
		std::string command;
		float value;
		std::tie(command, value) = bindingIt->second;
		m_CommandKeyboardValues[command][event.KeyCode] = value;
		PublishCommand(1, command, GetCommandTotalValue(command));
	}

	return true;
}

bool Systems::InputSystem::OnKeyUp(const Events::KeyUp &event)
{
	auto range = m_KeyBindings.equal_range(event.KeyCode);
	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
		std::string command;
		float value;
		std::tie(command, value) = bindingIt->second;
		m_CommandKeyboardValues[command][event.KeyCode] = 0;
		PublishCommand(1, command, GetCommandTotalValue(command));;
	}

	return true;
}

bool Systems::InputSystem::OnMousePress(const Events::MousePress &event)
{
	auto range = m_MouseButtonBindings.equal_range(event.Button);
	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
		std::string command;
		float value;
		std::tie(command, value) = bindingIt->second;
		m_CommandMouseButtonValues[command][event.Button] = value;
		PublishCommand(1, command, GetCommandTotalValue(command));
	}

	return true;
}

bool Systems::InputSystem::OnMouseRelease(const Events::MouseRelease &event)
{
	auto range = m_MouseButtonBindings.equal_range(event.Button);
	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
		std::string command;
		float value;
		std::tie(command, value) = bindingIt->second;
		m_CommandMouseButtonValues[command][event.Button] = 0;
		PublishCommand(1, command, GetCommandTotalValue(command));
	}

	return true;
}

bool Systems::InputSystem::OnGamepadAxis(const Events::GamepadAxis &event)
{
	auto range = m_GamepadAxisBindings.equal_range(event.Axis);
	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
		std::string command;
		float value;
		std::tie(command, value) = bindingIt->second;
		m_CommandGamepadAxisValues[command][event.Axis] = event.Value * value;
		PublishCommand(event.GamepadID + 1, command, GetCommandTotalValue(command));
	}

	return true;
}

bool Systems::InputSystem::OnGamepadButtonDown(const Events::GamepadButtonDown &event)
{
	auto range = m_GamepadButtonBindings.equal_range(event.Button);
	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
		std::string command;
		float value;
		std::tie(command, value) = bindingIt->second;
		m_CommandGamepadButtonValues[command][event.Button] = value;
		PublishCommand(event.GamepadID + 1, command, GetCommandTotalValue(command));
	}

	return true;
}

bool Systems::InputSystem::OnGamepadButtonUp(const Events::GamepadButtonUp &event)
{
	auto range = m_GamepadButtonBindings.equal_range(event.Button);
	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
		std::string command;
		float value;
		std::tie(command, value) = bindingIt->second;
		m_CommandGamepadButtonValues[command][event.Button] = 0;
		PublishCommand(event.GamepadID + 1, command, GetCommandTotalValue(command));
	}

	return true;
}

bool Systems::InputSystem::OnBindKey(const Events::BindKey &event)
{
	if (event.Command.empty()) {
		return false;
	}

	m_KeyBindings.insert(std::make_pair(event.KeyCode, std::make_tuple(event.Command, event.Value)));
	LOG_DEBUG("Input: Bound key %i to %s", event.KeyCode, event.Command.c_str());

	return true;
}

bool Systems::InputSystem::OnBindMouseButton(const Events::BindMouseButton &event)
{
	if (event.Command.empty()) {
		return false;
	}

	m_MouseButtonBindings.insert(std::make_pair(event.Button, std::make_tuple(event.Command, event.Value)));
	LOG_DEBUG("Input: Bound mouse button %i to %s", event.Button, event.Command.c_str());

	return true;
}

bool Systems::InputSystem::OnBindGamepadAxis(const Events::BindGamepadAxis &event)
{
	if (event.Command.empty()) {
		return false;
	}

	m_GamepadAxisBindings.insert(std::make_pair(event.Axis, std::make_tuple(event.Command, event.Value)));
	LOG_DEBUG("Input: Bound gamepad axis %i to %s", event.Axis, event.Command.c_str());

	return true;
}

bool Systems::InputSystem::OnBindGamepadButton(const Events::BindGamepadButton &event)
{
	if (event.Command.empty()) {
		return false;
	}

	m_GamepadButtonBindings.insert(std::make_pair(event.Button, std::make_tuple(event.Command, event.Value)));
	LOG_DEBUG("Input: Bound gamepad axis %i to %s", event.Button, event.Command.c_str());

	return true;
}

float Systems::InputSystem::GetCommandTotalValue(std::string command)
{
	float value = 0.f;

	auto keyboardIt = m_CommandKeyboardValues.find(command);
	if (keyboardIt != m_CommandKeyboardValues.end()) {
		for (auto &key : keyboardIt->second) {
			value += key.second;
		}
	}

	auto mouseButtonIt = m_CommandMouseButtonValues.find(command);
	if (mouseButtonIt != m_CommandMouseButtonValues.end()) {
		for (auto &button : mouseButtonIt->second) {
			value += button.second;
		}
	}

	auto gamepadAxisIt = m_CommandGamepadAxisValues.find(command);
	if (gamepadAxisIt != m_CommandGamepadAxisValues.end()) {
		for (auto &axis : gamepadAxisIt->second) {
			value += axis.second;
		}
	}

	auto gamepadButtonIt = m_CommandGamepadButtonValues.find(command);
	if (gamepadButtonIt != m_CommandGamepadButtonValues.end()) {
		for (auto &button : gamepadButtonIt->second) {
			value += button.second;
		}
	}

	return std::max(-1.f, std::min(value, 1.f));
}

void Systems::InputSystem::PublishCommand(int playerID, std::string command, float value)
{
	Events::InputCommand e;
	e.PlayerID = playerID;
	e.Command = command;
	e.Value = value;
	EventBroker->Publish(e);

	LOG_DEBUG("Input: Published command %s=%f for player %i", e.Command.c_str(), e.Value, playerID);
}
