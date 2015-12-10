#include "Input/InputProxy.h"
#include "Core/World.h"

//InputProxy::InputProxy(EventBroker* eventBroker)
//    : m_EventBroker(eventBroker)
//{
//	// Subscribe to events
//	EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &InputProxy::OnMousePress);
//	EVENT_SUBSCRIBE_MEMBER(m_EMouseRelease, &InputProxy::OnMouseRelease);
//	EVENT_SUBSCRIBE_MEMBER(m_EGamepadAxis, &InputProxy::OnGamepadAxis);
//	EVENT_SUBSCRIBE_MEMBER(m_EGamepadButtonDown, &InputProxy::OnGamepadButtonDown);
//	EVENT_SUBSCRIBE_MEMBER(m_EGamepadButtonUp, &InputProxy::OnGamepadButtonUp);
//	EVENT_SUBSCRIBE_MEMBER(m_EBindKey, &InputProxy::OnBindKey);
//	EVENT_SUBSCRIBE_MEMBER(m_EBindMouseButton, &InputProxy::OnBindMouseButton);
//	EVENT_SUBSCRIBE_MEMBER(m_EBindGamepadAxis, &InputProxy::OnBindGamepadAxis);
//	EVENT_SUBSCRIBE_MEMBER(m_EBindGamepadButton, &InputProxy::OnBindGamepadButton);
//    
//    SteamController()->Init();
//    
//}
//
//void InputProxy::Update(double dt)
//{
//    std::array<ControllerHandle_t, STEAM_CONTROLLER_MAX_COUNT> controllers;
//    int numControllers = SteamController()->GetConnectedControllers(controllers.data());
//
//    ControllerDigitalActionHandle_t debug_reload_handle = SteamController()->GetDigitalActionHandle("debug_reload");
//    SteamController()->
//}
//
//bool InputProxy::OnKeyDown(const Events::KeyDown &event)
//{
//	auto range = m_KeyBindings.equal_range(event.KeyCode);
//	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
//		std::string command;
//		float value;
//		std::tie(command, value) = bindingIt->second;
//		m_CommandKeyboardValues[command][event.KeyCode] = value;
//		PublishCommand(1, command, GetCommandTotalValue(command));
//	}
//
//	return true;
//}
//
//bool InputProxy::OnKeyUp(const Events::KeyUp &event)
//{
//	auto range = m_KeyBindings.equal_range(event.KeyCode);
//	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
//		std::string command;
//		float value;
//		std::tie(command, value) = bindingIt->second;
//		m_CommandKeyboardValues[command][event.KeyCode] = 0;
//		PublishCommand(1, command, GetCommandTotalValue(command));;
//	}
//
//	return true;
//}
//
//bool InputProxy::OnMousePress(const Events::MousePress &event)
//{
//	auto range = m_MouseButtonBindings.equal_range(event.Button);
//	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
//		std::string command;
//		float value;
//		std::tie(command, value) = bindingIt->second;
//		m_CommandMouseButtonValues[command][event.Button] = value;
//		PublishCommand(1, command, GetCommandTotalValue(command));
//	}
//
//	return true;
//}
//
//bool InputProxy::OnMouseRelease(const Events::MouseRelease &event)
//{
//	auto range = m_MouseButtonBindings.equal_range(event.Button);
//	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
//		std::string command;
//		float value;
//		std::tie(command, value) = bindingIt->second;
//		m_CommandMouseButtonValues[command][event.Button] = 0;
//		PublishCommand(1, command, GetCommandTotalValue(command));
//	}
//
//	return true;
//}
//
//bool InputProxy::OnGamepadAxis(const Events::GamepadAxis &event)
//{
//	auto range = m_GamepadAxisBindings.equal_range(event.Axis);
//	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
//		std::string command;
//		float value;
//		std::tie(command, value) = bindingIt->second;
//		m_CommandGamepadAxisValues[command][event.Axis] = event.Value * value;
//		PublishCommand(event.GamepadID + 1, command, GetCommandTotalValue(command));
//	}
//
//	return true;
//}
//
//bool InputProxy::OnGamepadButtonDown(const Events::GamepadButtonDown &event)
//{
//	auto range = m_GamepadButtonBindings.equal_range(event.Button);
//	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
//		std::string command;
//		float value;
//		std::tie(command, value) = bindingIt->second;
//		m_CommandGamepadButtonValues[command][event.Button] = value;
//		PublishCommand(event.GamepadID + 1, command, GetCommandTotalValue(command));
//	}
//
//	return true;
//}
//
//bool InputProxy::OnGamepadButtonUp(const Events::GamepadButtonUp &event)
//{
//	auto range = m_GamepadButtonBindings.equal_range(event.Button);
//	for (auto bindingIt = range.first; bindingIt != range.second; bindingIt++) {
//		std::string command;
//		float value;
//		std::tie(command, value) = bindingIt->second;
//		m_CommandGamepadButtonValues[command][event.Button] = 0;
//		PublishCommand(event.GamepadID + 1, command, GetCommandTotalValue(command));
//	}
//
//	return true;
//}
//
//bool InputProxy::OnBindKey(const Events::BindKey &event)
//{
//	if (event.Command.empty()) {
//		return false;
//	}
//
//	m_KeyBindings.insert(std::make_pair(event.KeyCode, std::make_tuple(event.Command, event.Value)));
//	LOG_DEBUG("Input: Bound key %i to %s", event.KeyCode, event.Command.c_str());
//
//	return true;
//}
//
//bool InputProxy::OnBindMouseButton(const Events::BindMouseButton &event)
//{
//	if (event.Command.empty()) {
//		return false;
//	}
//
//	m_MouseButtonBindings.insert(std::make_pair(event.Button, std::make_tuple(event.Command, event.Value)));
//	LOG_DEBUG("Input: Bound mouse button %i to %s", event.Button, event.Command.c_str());
//
//	return true;
//}
//
//bool InputProxy::OnBindGamepadAxis(const Events::BindGamepadAxis &event)
//{
//	if (event.Command.empty()) {
//		return false;
//	}
//
//	m_GamepadAxisBindings.insert(std::make_pair(event.Axis, std::make_tuple(event.Command, event.Value)));
//	LOG_DEBUG("Input: Bound gamepad axis %i to %s", event.Axis, event.Command.c_str());
//
//	return true;
//}
//
//bool InputProxy::OnBindGamepadButton(const Events::BindGamepadButton &event)
//{
//	if (event.Command.empty()) {
//		return false;
//	}
//
//	m_GamepadButtonBindings.insert(std::make_pair(event.Button, std::make_tuple(event.Command, event.Value)));
//	LOG_DEBUG("Input: Bound gamepad axis %i to %s", event.Button, event.Command.c_str());
//
//	return true;
//}
//
//float InputProxy::GetCommandTotalValue(std::string command)
//{
//	float value = 0.f;
//
//	auto keyboardIt = m_CommandKeyboardValues.find(command);
//	if (keyboardIt != m_CommandKeyboardValues.end()) {
//		for (auto &key : keyboardIt->second) {
//			value += key.second;
//		}
//	}
//
//	auto mouseButtonIt = m_CommandMouseButtonValues.find(command);
//	if (mouseButtonIt != m_CommandMouseButtonValues.end()) {
//		for (auto &button : mouseButtonIt->second) {
//			value += button.second;
//		}
//	}
//
//	auto gamepadAxisIt = m_CommandGamepadAxisValues.find(command);
//	if (gamepadAxisIt != m_CommandGamepadAxisValues.end()) {
//		for (auto &axis : gamepadAxisIt->second) {
//			value += axis.second;
//		}
//	}
//
//	auto gamepadButtonIt = m_CommandGamepadButtonValues.find(command);
//	if (gamepadButtonIt != m_CommandGamepadButtonValues.end()) {
//		for (auto &button : gamepadButtonIt->second) {
//			value += button.second;
//		}
//	}
//
//	return std::max(-1.f, std::min(value, 1.f));
//}
//
//void InputProxy::PublishCommand(int playerID, std::string command, float value)
//{
//	Events::InputCommand e;
//	e.PlayerID = playerID;
//	e.Command = command;
//	e.Value = value;
//	m_EventBroker->Publish(e);
//
//	LOG_DEBUG("Input: Published command %s=%f for player %i", e.Command.c_str(), e.Value, playerID);
//}
