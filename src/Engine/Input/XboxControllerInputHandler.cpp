#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")
#pragma comment(lib, "Xinput9_1_0.lib")
#include "Input/XboxControllerInputHandler.h"

XboxControllerInputHandler::XboxControllerInputHandler(EventBroker* eventBroker, InputProxy* inputProxy) 
    : InputHandler(eventBroker, inputProxy)
{
    m_OriginButtons["GamepadUp"] = XINPUT_GAMEPAD_DPAD_UP;
    m_OriginButtons["GamepadDown"] = XINPUT_GAMEPAD_DPAD_DOWN;
    m_OriginButtons["GamepadLeft"] = XINPUT_GAMEPAD_DPAD_LEFT;
    m_OriginButtons["GamepadRight"] = XINPUT_GAMEPAD_DPAD_RIGHT;
    m_OriginButtons["GamepadStart"] = XINPUT_GAMEPAD_START;
    m_OriginButtons["GamepadBack"] = XINPUT_GAMEPAD_BACK;
    m_OriginButtons["GamepadLeftStick"] = XINPUT_GAMEPAD_LEFT_THUMB;
    m_OriginButtons["GamepadRightStick"] = XINPUT_GAMEPAD_RIGHT_THUMB;
    m_OriginButtons["GamepadLeftBumper"] = XINPUT_GAMEPAD_LEFT_SHOULDER;
    m_OriginButtons["GamepadRightBumper"] = XINPUT_GAMEPAD_RIGHT_SHOULDER;
    m_OriginButtons["GamepadA"] = XINPUT_GAMEPAD_A;
    m_OriginButtons["GamepadB"] = XINPUT_GAMEPAD_B;
    m_OriginButtons["GamepadX"] = XINPUT_GAMEPAD_X;
    m_OriginButtons["GamepadY"] = XINPUT_GAMEPAD_Y;
    m_OriginAxes["GamepadLeftX"] = 0;
    m_OriginAxes["GamepadLeftY"] = 1;
    m_OriginAxes["GamepadRightX"] = 2;
    m_OriginAxes["GamepadRightY"] = 3;
    m_OriginAxes["GamepadLeftTrigger"] = 4;
    m_OriginAxes["GamepadRightTrigger"] = 5;
}

bool XboxControllerInputHandler::BindOrigin(std::string origin, std::string command, float value)
{
    bool result = false;

    auto originIt = m_OriginButtons.find(origin);
    if (originIt != m_OriginButtons.end()) {
        int button = originIt->second;
        m_ButtonBindings[button] = std::make_tuple(command, value);
        result = true;
    }

    auto originAxis = m_OriginAxes.find(origin);
    if (originAxis != m_OriginAxes.end()) {
        char axis = originAxis->second;
        float multiplier = 0.5f;
        //// Sensitivity
        //multiplier *= ResourceManager::Load<ConfigFile>("Input.ini")->Get<float>("Mouse.Sensitivity", 1.f);
        //if (axis == 'Y') {
        //    if (ResourceManager::Load<ConfigFile>("Input.ini")->Get<bool>("Mouse.InvertPitch", false)) {
        //        multiplier *= -1.f;
        //    }
        //}
        m_AxisBindings[axis] = std::make_tuple(command, value * multiplier);
        result = true;
    }

    return result;
}

void XboxControllerInputHandler::Update(double dt)
{
    DWORD dwResult;
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        XINPUT_STATE state = { 0 };
        // Simply get the state of the controller from XInput.
        dwResult = XInputGetState(i, &state);
        if (dwResult == 0) {
            if (std::abs(state.Gamepad.sThumbLX) <= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
                state.Gamepad.sThumbLX = 0;
            if (std::abs(state.Gamepad.sThumbLY) <= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
                state.Gamepad.sThumbLY = 0;
            if (std::abs(state.Gamepad.sThumbRX) <= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                state.Gamepad.sThumbRX = 0;
            if (std::abs(state.Gamepad.sThumbRY) <= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                state.Gamepad.sThumbRY = 0;
            if (std::abs(state.Gamepad.bLeftTrigger) <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
                state.Gamepad.bLeftTrigger = 0;
            if (std::abs(state.Gamepad.bRightTrigger) <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
                state.Gamepad.bRightTrigger = 0;

            //m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::LeftX)] = state.Gamepad.sThumbLX / 32767.f;
            //m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::LeftY)] = state.Gamepad.sThumbLY / 32767.f;
            //m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::RightX)] = state.Gamepad.sThumbRX / 32767.f;
            //m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::RightY)] = state.Gamepad.sThumbRY / 32767.f;
            //m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::LeftTrigger)] = state.Gamepad.bLeftTrigger / 255.f;
            //m_CurrentGamepadAxisState[i][static_cast<int>(Gamepad::Axis::RightTrigger)] = state.Gamepad.bRightTrigger / 255.f;
            //PublishGamepadAxisIfChanged(i, Gamepad::Axis::LeftX);
            //PublishGamepadAxisIfChanged(i, Gamepad::Axis::LeftY);
            //PublishGamepadAxisIfChanged(i, Gamepad::Axis::RightX);
            //PublishGamepadAxisIfChanged(i, Gamepad::Axis::RightY);
            //PublishGamepadAxisIfChanged(i, Gamepad::Axis::LeftTrigger);
            //PublishGamepadAxisIfChanged(i, Gamepad::Axis::RightTrigger);

            for (auto& pair : m_OriginAxes) {
                const std::string& origin = pair.first;
                int axis = pair.second;
                auto& binding = m_AxisBindings.find(axis);
                if (binding == m_AxisBindings.end()) {
                    continue;
                }

                std::string command;
                float value;
                std::tie(command, value) = binding->second;

                Events::InputCommand e;
                e.PlayerID = -1;
                e.Command = command;
                e.Value = value * getAxisValue(state, axis);
                m_InputProxy->Publish(e);
            }

            for (auto& pair : m_OriginButtons) {
                auto& binding = m_ButtonBindings.find(pair.second);
                if (binding == m_ButtonBindings.end()) {
                    continue;
                }

                std::string command;
                float value;
                std::tie(command, value) = binding->second;
                bool pressed = static_cast<bool>(state.Gamepad.wButtons & binding->first);
                m_CommandValues[command] = (pressed) ? value : 0.f;
            }

            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Up)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Down)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Left)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Right)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Start)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Back)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::LeftThumb)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::RightThumb)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::LeftShoulder)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::RightShoulder)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::A)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::B)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::X)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
            //m_CurrentGamepadButtonState[i][static_cast<int>(Gamepad::Button::Y)] = static_cast<bool>(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::Up);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::Down);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::Left);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::Right);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::Start);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::Back);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::LeftThumb);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::RightThumb);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::LeftShoulder);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::RightShoulder);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::A);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::B);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::X);
            //PublishGamepadButtonIfChanged(i, Gamepad::Button::Y);
        }
    }
}

float XboxControllerInputHandler::GetCommandValue(std::string command)
{
    auto it = m_CommandValues.find(command);
    if (it != m_CommandValues.end()) {
        return m_CommandValues[command];
    } else {
        return 0.f;
    }
}

float XboxControllerInputHandler::getAxisValue(XINPUT_STATE& state, int axis)
{
    switch (axis) {
    case 0:
        return state.Gamepad.sThumbLX / 32767.f;
    case 1:
        return state.Gamepad.sThumbLY / 32767.f;
    case 2:
        return state.Gamepad.sThumbRX / 32767.f;
    case 3:
        return state.Gamepad.sThumbRY / 32767.f;
    case 4:
        return state.Gamepad.bLeftTrigger / 255.f;
    case 5:
        return state.Gamepad.bRightTrigger / 255.f;
    }
}

