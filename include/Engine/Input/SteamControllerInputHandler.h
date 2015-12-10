#include <steam/steam_api.h>
#include "InputProxy.h"

class SteamControllerInputHandler : public InputHandler
{
public:
    SteamControllerInputHandler(EventBroker* eventBroker, InputProxy* inputProxy)
        : InputHandler(eventBroker, inputProxy)
    {
        SteamController()->Init();
    }

    ~SteamControllerInputHandler()
    {
        SteamController()->Shutdown();
    }

    void Update(double dt) override
    {
        std::array<ControllerHandle_t, STEAM_CONTROLLER_MAX_COUNT> controllers;
        int numControllers = SteamController()->GetConnectedControllers(controllers.data());
        for (int i = 0; i < numControllers; i++) {
            auto controllerHandle = controllers.at(i);
            auto actionSetHandle = SteamController()->GetActionSetHandle("InGameControls");
            //SteamController()->ShowBindingPanel(controllerHandle);
            SteamController()->ActivateActionSet(controllerHandle, actionSetHandle);
            for (auto& command : m_Commands) {
                Events::InputCommand ic;
                ic.PlayerID = i + 1;
                ic.Command = command.first;

                auto digitalActionHandle = m_DigitalActionHandles.at(ic.Command);
                auto handle = SteamController()->GetDigitalActionHandle("DebugReload");
                auto data = SteamController()->GetDigitalActionData(controllerHandle, handle);
                LOG_DEBUG("Controller %i, active %i, value %i", i, data.bActive, data.bState);
                if (data.bState) {
                    ic.Value = command.second;
                } else {
                    ic.Value = 0.f;
                }
                //m_InputProxy->Publish(ic);
            }
        }
        
    }

    bool BindOrigin(std::string origin, std::string command, float value) override
    {
        if (origin != "SteamController") {
            return false;
        }

        m_Commands[command] = value;
        m_DigitalActionHandles[command] = SteamController()->GetDigitalActionHandle(command.c_str());
        return true;
    }

private:
    std::map<std::string, float> m_Commands;
    std::map<std::string, ControllerDigitalActionHandle_t> m_DigitalActionHandles;

	//std::unordered_map<std::string, std::unordered_map<int, float>> m_CommandKeyboardValues; // command string -> keyboard key value for command
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
