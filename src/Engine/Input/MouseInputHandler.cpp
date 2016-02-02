#include "Input/MouseInputHandler.h"

MouseInputHandler::MouseInputHandler(EventBroker* eventBroker, InputProxy* inputProxy)
    : InputHandler(eventBroker, inputProxy)
{
    EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &MouseInputHandler::OnMousePress);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseRelease, &MouseInputHandler::OnMouseRelease);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseMove, &MouseInputHandler::OnMouseMove);

    m_OriginCodes["Mouse1"] = GLFW_MOUSE_BUTTON_1;
    m_OriginCodes["MouseLeft"] = GLFW_MOUSE_BUTTON_LEFT;
    m_OriginCodes["Mouse2"] = GLFW_MOUSE_BUTTON_2;
    m_OriginCodes["MouseRight"] = GLFW_MOUSE_BUTTON_RIGHT;
    m_OriginCodes["Mouse3"] = GLFW_MOUSE_BUTTON_3;
    m_OriginCodes["MouseMiddle"] = GLFW_MOUSE_BUTTON_MIDDLE;
    m_OriginCodes["Mouse4"] = GLFW_MOUSE_BUTTON_4;
    m_OriginCodes["Mouse5"] = GLFW_MOUSE_BUTTON_5;
    m_OriginCodes["Mouse6"] = GLFW_MOUSE_BUTTON_6;
    m_OriginCodes["Mouse7"] = GLFW_MOUSE_BUTTON_7;
    m_OriginCodes["Mouse8"] = GLFW_MOUSE_BUTTON_8;

    m_OriginAxes["MouseX"] = 'X';
    m_OriginAxes["MouseY"] = 'Y';
}

bool MouseInputHandler::BindOrigin(std::string origin, std::string command, float value)
{
    auto originCode = m_OriginCodes.find(origin);
    if (originCode != m_OriginCodes.end()) {
        int code = originCode->second;
        m_Bindings[code] = std::make_tuple(command, value);
        return true;
    }

    auto originAxis = m_OriginAxes.find(origin);
    if (originAxis != m_OriginAxes.end()) {
        char axis = originAxis->second;
        float multiplier = 1.f;
        // Sensitivity
        multiplier *= ResourceManager::Load<ConfigFile>("Input.ini")->Get<float>("Mouse.Sensitivity", 1.f);
        if (axis == 'Y') {
            if (ResourceManager::Load<ConfigFile>("Input.ini")->Get<bool>("Mouse.InvertPitch", false)) {
                multiplier *= -1.f;
            }
        }
        m_Axes[axis] = std::make_tuple(command, value * multiplier);
        return true;
    }

    return false;
}

float MouseInputHandler::GetCommandValue(std::string command)
{
    auto it = m_CommandValues.find(command);
    if (it != m_CommandValues.end()) {
        return m_CommandValues[command];
    } else {
        return 0.f;
    }
}

bool MouseInputHandler::OnMousePress(const Events::MousePress& e)
{
    auto it = m_Bindings.find(e.Button);
    if (it == m_Bindings.end()) {
        return false;
    }

    std::string command;
    float value;
    std::tie(command, value) = it->second;
    m_CommandValues[command] += value;

    return true;
}

bool MouseInputHandler::OnMouseRelease(const Events::MouseRelease& e)
{
    auto it = m_Bindings.find(e.Button);
    if (it == m_Bindings.end()) {
        return false;
    }

    std::string command;
    float value;
    std::tie(command, value) = it->second;
    m_CommandValues[command] -= value;

    return true;
}

bool MouseInputHandler::OnMouseMove(const Events::MouseMove& e)
{
    if (std::abs(e.DeltaX) > 0) {
        auto it = m_Axes.find('X');
        if (it != m_Axes.end()) {
            Events::InputCommand ic;
            ic.PlayerID = -1;
            std::tie(ic.Command, ic.Value) = it->second;
            ic.Value *= static_cast<float>(e.DeltaX);
            m_InputProxy->Publish(ic);
        }
    }
    
    if (std::abs(e.DeltaY) > 0) {
        auto it = m_Axes.find('Y');
        if (it != m_Axes.end()) {
            Events::InputCommand ic;
            ic.PlayerID = -1;
            std::tie(ic.Command, ic.Value) = it->second;
            ic.Value *= static_cast<float>(e.DeltaY);
            m_InputProxy->Publish(ic);
        }
    }

    return true;
}

bool MouseInputHandler::hasOrigin(std::string origin)
{
    if (m_OriginCodes.find(origin) == m_OriginCodes.end()) {
        return false;
    }
    if (m_OriginAxes.find(origin) == m_OriginAxes.end()) {
        return false;
    }
    return true;
}