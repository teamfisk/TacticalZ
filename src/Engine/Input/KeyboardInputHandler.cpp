#include "Input/KeyboardInputHandler.h"

KeyboardInputHandler::KeyboardInputHandler(EventBroker* eventBroker, InputProxy* inputProxy) : InputHandler(eventBroker, inputProxy)
{
    EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &KeyboardInputHandler::OnKeyDown);
    EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &KeyboardInputHandler::OnKeyUp);

    m_OriginKeyCodes["Space"] = GLFW_KEY_SPACE;
    m_OriginKeyCodes["Apostrophe"] = GLFW_KEY_APOSTROPHE;
    m_OriginKeyCodes["Comma"] = GLFW_KEY_COMMA;
    m_OriginKeyCodes["Minus"] = GLFW_KEY_MINUS;
    m_OriginKeyCodes["Period"] = GLFW_KEY_PERIOD;
    m_OriginKeyCodes["Slash"] = GLFW_KEY_SLASH;
    m_OriginKeyCodes["0"] = GLFW_KEY_0;
    m_OriginKeyCodes["1"] = GLFW_KEY_1;
    m_OriginKeyCodes["2"] = GLFW_KEY_2;
    m_OriginKeyCodes["3"] = GLFW_KEY_3;
    m_OriginKeyCodes["4"] = GLFW_KEY_4;
    m_OriginKeyCodes["5"] = GLFW_KEY_5;
    m_OriginKeyCodes["6"] = GLFW_KEY_6;
    m_OriginKeyCodes["7"] = GLFW_KEY_7;
    m_OriginKeyCodes["8"] = GLFW_KEY_8;
    m_OriginKeyCodes["9"] = GLFW_KEY_9;
    m_OriginKeyCodes["Semicolon"] = GLFW_KEY_SEMICOLON;
    m_OriginKeyCodes["Equal"] = GLFW_KEY_EQUAL;
    m_OriginKeyCodes["A"] = GLFW_KEY_A;
    m_OriginKeyCodes["B"] = GLFW_KEY_B;
    m_OriginKeyCodes["C"] = GLFW_KEY_C;
    m_OriginKeyCodes["D"] = GLFW_KEY_D;
    m_OriginKeyCodes["E"] = GLFW_KEY_E;
    m_OriginKeyCodes["F"] = GLFW_KEY_F;
    m_OriginKeyCodes["G"] = GLFW_KEY_G;
    m_OriginKeyCodes["H"] = GLFW_KEY_H;
    m_OriginKeyCodes["I"] = GLFW_KEY_I;
    m_OriginKeyCodes["J"] = GLFW_KEY_J;
    m_OriginKeyCodes["K"] = GLFW_KEY_K;
    m_OriginKeyCodes["L"] = GLFW_KEY_L;
    m_OriginKeyCodes["M"] = GLFW_KEY_M;
    m_OriginKeyCodes["N"] = GLFW_KEY_N;
    m_OriginKeyCodes["O"] = GLFW_KEY_O;
    m_OriginKeyCodes["P"] = GLFW_KEY_P;
    m_OriginKeyCodes["Q"] = GLFW_KEY_Q;
    m_OriginKeyCodes["R"] = GLFW_KEY_R;
    m_OriginKeyCodes["S"] = GLFW_KEY_S;
    m_OriginKeyCodes["T"] = GLFW_KEY_T;
    m_OriginKeyCodes["U"] = GLFW_KEY_U;
    m_OriginKeyCodes["V"] = GLFW_KEY_V;
    m_OriginKeyCodes["W"] = GLFW_KEY_W;
    m_OriginKeyCodes["X"] = GLFW_KEY_X;
    m_OriginKeyCodes["Y"] = GLFW_KEY_Y;
    m_OriginKeyCodes["Z"] = GLFW_KEY_Z;
    m_OriginKeyCodes["LeftBracket"] = GLFW_KEY_LEFT_BRACKET;
    m_OriginKeyCodes["Backslash"] = GLFW_KEY_BACKSLASH;
    m_OriginKeyCodes["RightBracket"] = GLFW_KEY_RIGHT_BRACKET;
    m_OriginKeyCodes["Accent"] = GLFW_KEY_GRAVE_ACCENT;
    m_OriginKeyCodes["W1"] = GLFW_KEY_WORLD_1;
    m_OriginKeyCodes["W2"] = GLFW_KEY_WORLD_2;
    m_OriginKeyCodes["Escape"] = GLFW_KEY_ESCAPE;
    m_OriginKeyCodes["Enter"] = GLFW_KEY_ENTER;
    m_OriginKeyCodes["Tab"] = GLFW_KEY_TAB;
    m_OriginKeyCodes["Backspace"] = GLFW_KEY_BACKSPACE;
    m_OriginKeyCodes["Insert"] = GLFW_KEY_INSERT;
    m_OriginKeyCodes["Delete"] = GLFW_KEY_DELETE;
    m_OriginKeyCodes["Right"] = GLFW_KEY_RIGHT;
    m_OriginKeyCodes["Left"] = GLFW_KEY_LEFT;
    m_OriginKeyCodes["Down"] = GLFW_KEY_DOWN;
    m_OriginKeyCodes["Up"] = GLFW_KEY_UP;
    m_OriginKeyCodes["PgUp"] = GLFW_KEY_PAGE_UP;
    m_OriginKeyCodes["PgDn"] = GLFW_KEY_PAGE_DOWN;
    m_OriginKeyCodes["Home"] = GLFW_KEY_HOME;
    m_OriginKeyCodes["End"] = GLFW_KEY_END;
    m_OriginKeyCodes["CapsLock"] = GLFW_KEY_CAPS_LOCK;
    m_OriginKeyCodes["ScrollLock"] = GLFW_KEY_SCROLL_LOCK;
    m_OriginKeyCodes["NumLock"] = GLFW_KEY_NUM_LOCK;
    m_OriginKeyCodes["PrintScreen"] = GLFW_KEY_PRINT_SCREEN;
    m_OriginKeyCodes["Pause"] = GLFW_KEY_PAUSE;
    m_OriginKeyCodes["F1"] = GLFW_KEY_F1;
    m_OriginKeyCodes["F2"] = GLFW_KEY_F2;
    m_OriginKeyCodes["F3"] = GLFW_KEY_F3;
    m_OriginKeyCodes["F4"] = GLFW_KEY_F4;
    m_OriginKeyCodes["F5"] = GLFW_KEY_F5;
    m_OriginKeyCodes["F6"] = GLFW_KEY_F6;
    m_OriginKeyCodes["F7"] = GLFW_KEY_F7;
    m_OriginKeyCodes["F8"] = GLFW_KEY_F8;
    m_OriginKeyCodes["F9"] = GLFW_KEY_F9;
    m_OriginKeyCodes["F10"] = GLFW_KEY_F10;
    m_OriginKeyCodes["F11"] = GLFW_KEY_F11;
    m_OriginKeyCodes["F12"] = GLFW_KEY_F12;
    m_OriginKeyCodes["F13"] = GLFW_KEY_F13;
    m_OriginKeyCodes["F14"] = GLFW_KEY_F14;
    m_OriginKeyCodes["F15"] = GLFW_KEY_F15;
    m_OriginKeyCodes["F16"] = GLFW_KEY_F16;
    m_OriginKeyCodes["F17"] = GLFW_KEY_F17;
    m_OriginKeyCodes["F18"] = GLFW_KEY_F18;
    m_OriginKeyCodes["F19"] = GLFW_KEY_F19;
    m_OriginKeyCodes["F20"] = GLFW_KEY_F20;
    m_OriginKeyCodes["F21"] = GLFW_KEY_F21;
    m_OriginKeyCodes["F22"] = GLFW_KEY_F22;
    m_OriginKeyCodes["F23"] = GLFW_KEY_F23;
    m_OriginKeyCodes["F24"] = GLFW_KEY_F24;
    m_OriginKeyCodes["F25"] = GLFW_KEY_F25;
    m_OriginKeyCodes["KP0"] = GLFW_KEY_KP_0;
    m_OriginKeyCodes["KP1"] = GLFW_KEY_KP_1;
    m_OriginKeyCodes["KP2"] = GLFW_KEY_KP_2;
    m_OriginKeyCodes["KP3"] = GLFW_KEY_KP_3;
    m_OriginKeyCodes["KP4"] = GLFW_KEY_KP_4;
    m_OriginKeyCodes["KP5"] = GLFW_KEY_KP_5;
    m_OriginKeyCodes["KP6"] = GLFW_KEY_KP_6;
    m_OriginKeyCodes["KP7"] = GLFW_KEY_KP_7;
    m_OriginKeyCodes["KP8"] = GLFW_KEY_KP_8;
    m_OriginKeyCodes["KP9"] = GLFW_KEY_KP_9;
    m_OriginKeyCodes["KPDecimal"] = GLFW_KEY_KP_DECIMAL;
    m_OriginKeyCodes["KPDivide"] = GLFW_KEY_KP_DIVIDE;
    m_OriginKeyCodes["KPMultiply"] = GLFW_KEY_KP_MULTIPLY;
    m_OriginKeyCodes["KPSubtract"] = GLFW_KEY_KP_SUBTRACT;
    m_OriginKeyCodes["KPAdd"] = GLFW_KEY_KP_ADD;
    m_OriginKeyCodes["KPEnter"] = GLFW_KEY_KP_ENTER;
    m_OriginKeyCodes["KPEqual"] = GLFW_KEY_KP_EQUAL;
    m_OriginKeyCodes["LeftShift"] = GLFW_KEY_LEFT_SHIFT;
    m_OriginKeyCodes["LeftControl"] = GLFW_KEY_LEFT_CONTROL;
    m_OriginKeyCodes["LeftAlt"] = GLFW_KEY_LEFT_ALT;
    m_OriginKeyCodes["LeftSuper"] = GLFW_KEY_LEFT_SUPER;
    m_OriginKeyCodes["RightShift"] = GLFW_KEY_RIGHT_SHIFT;
    m_OriginKeyCodes["RightControl"] = GLFW_KEY_RIGHT_CONTROL;
    m_OriginKeyCodes["RightAlt"] = GLFW_KEY_RIGHT_ALT;
    m_OriginKeyCodes["RightSuper"] = GLFW_KEY_RIGHT_SUPER;
    m_OriginKeyCodes["Menu"] = GLFW_KEY_MENU;
}

bool KeyboardInputHandler::BindOrigin(std::string origin, std::string command, float value)
{
    auto originIt = m_OriginKeyCodes.find(origin);
    if (originIt == m_OriginKeyCodes.end()) {
        return false;
    }

    int keyCode = originIt->second;
    m_KeyBindings[keyCode] = std::make_tuple(command, value);
    return true;
}

bool KeyboardInputHandler::OnKeyDown(const Events::KeyDown& e)
{
    auto it = m_KeyBindings.find(e.KeyCode);
    if (it == m_KeyBindings.end()) {
        return false;
    }

    Events::InputCommand ic;
    ic.PlayerID = 0;
    std::tie(ic.Command, ic.Value) = it->second;
    m_InputProxy->Publish(ic);

    return true;
}

bool KeyboardInputHandler::OnKeyUp(const Events::KeyUp& e)
{
    auto it = m_KeyBindings.find(e.KeyCode);
    if (it == m_KeyBindings.end()) {
        return false;
    }

    Events::InputCommand ic;
    ic.PlayerID = 0;
    std::tie(ic.Command, std::ignore) = it->second;
    ic.Value = 0;
    m_InputProxy->Publish(ic);

    return true;
}

