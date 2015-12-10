#include <GLFW/glfw3.h>
#include "InputProxy.h"
#include "Core/EKeyDown.h"
#include "Core/EKeyUp.h"

class KeyboardInputHandler : public InputHandler
{
public:
    KeyboardInputHandler(EventBroker* eventBroker, InputProxy* inputProxy)
        : InputHandler(eventBroker, inputProxy)
    {
        EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &KeyboardInputHandler::OnKeyDown);
        EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &KeyboardInputHandler::OnKeyUp);

        m_OriginKeyCodes["R"] = GLFW_KEY_R;
    }

    bool BindOrigin(std::string origin, std::string command, float value) override
    {
        auto originIt = m_OriginKeyCodes.find(origin);
        if (originIt == m_OriginKeyCodes.end()) {
            return false;
        }

        int keyCode = originIt->second;
        m_KeyBindings[keyCode] = std::make_tuple(command, value);
        return true;
    }

private:
    std::unordered_map<std::string, int> m_OriginKeyCodes;
	std::unordered_map<int, std::tuple<std::string, float>> m_KeyBindings; // GLFW_KEY... -> command string & value

	EventRelay<InputHandler, Events::KeyDown> m_EKeyDown;
    bool OnKeyDown(const Events::KeyDown& e)
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

	EventRelay<InputHandler, Events::KeyUp> m_EKeyUp;
	bool OnKeyUp(const Events::KeyUp& e)
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

};