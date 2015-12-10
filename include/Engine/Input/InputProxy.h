#ifndef InputSystem_h__
#define InputSystem_h__

#include <array>
#include <unordered_map>

#include <steam/steam_api.h>

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
#include "EBindOrigin.h"

class InputProxy;

class InputHandler
{
public:
    InputHandler(EventBroker* eventBroker, InputProxy* inputProxy)
        : m_EventBroker(eventBroker)
        , m_InputProxy(inputProxy)
    { }

    virtual void Update(double dt) { }
    virtual bool BindOrigin(std::string origin, std::string command, float value) = 0;

protected:
    EventBroker* m_EventBroker;
    InputProxy* m_InputProxy;
};

class InputProxy
{
public:
    InputProxy(EventBroker* eventBroker)
        : m_EventBroker(eventBroker)
    {
        EVENT_SUBSCRIBE_MEMBER(m_EBindOrigin, &InputProxy::OnBindOrigin);
    }

	void Update(double dt)
    {
        m_EventBroker->Process<InputProxy>();
        m_EventBroker->Process<InputHandler>();
        for (auto& handler : m_Handlers) {
            handler->Update(dt);
        }
    }

    void Process()
    {
        // Accumulate the input values of all unique commands published by input handlers
        for (auto& pair : m_CommandQueue) {
            Events::InputCommand e;
            e.PlayerID = pair.first.first;
            e.Command = pair.first.second;
            e.Value = 0;
            for (auto& value : pair.second) {
                e.Value += value;
            }
	        e.Value = std::max(-1.f, std::min(e.Value, 1.f));
            m_EventBroker->Publish(e);
            LOG_DEBUG("Input: Published command %s=%f for player %i", e.Command.c_str(), e.Value, e.PlayerID);
        }
        m_CommandQueue.clear();
    }

    template <typename T>
    void AddHandler()
    {
        m_Handlers.push_back(new T(m_EventBroker, this));
    }

    void Publish(const Events::InputCommand& e)
    {
        auto key = std::make_pair(e.PlayerID, e.Command);
        m_CommandQueue[key].push_back(e.Value);
    }

protected:
    EventBroker* m_EventBroker;
    std::vector<InputHandler*> m_Handlers;
    // Represents every unique command (has of PlayerID & Command) and all values reported for that command
    std::map<std::pair<unsigned int, std::string>, std::vector<float>> m_CommandQueue;

	EventRelay<InputProxy, Events::BindOrigin> m_EBindOrigin;
	bool OnBindOrigin(const Events::BindOrigin& e)
    {
        bool originBound = false;
        for (auto& handler : m_Handlers) {
            bool result = handler->BindOrigin(e.Origin, e.Command, e.Value);
            if (result) {
                if (originBound) {
                    LOG_WARNING("Multiple handlers responded to binding input origin \"%s\"!", e.Origin.c_str());
                }
                originBound = true;
            }
        }

        if (!originBound) {
            LOG_ERROR("No input handler responded to binding input origin \"%s\"!", e.Origin.c_str());
        }

        return originBound;
    }
	//std::unordered_map<std::string, std::unordered_map<int, float>> m_CommandMouseButtonValues; // command string -> mouse button value for command
	//std::unordered_map<std::string, std::unordered_map<Gamepad::Axis, float, EnumClassHash>> m_CommandGamepadAxisValues; // command string -> gamepad axis value for command
	//std::unordered_map<std::string, std::unordered_map<Gamepad::Button, float, EnumClassHash>> m_CommandGamepadButtonValues; // command string -> gamepad button value for command
	//// Input binding tables
	//std::unordered_multimap<int, std::tuple<std::string, float>> m_MouseButtonBindings; // GLFW_MOUSE_BUTTON... -> command string
	//std::unordered_multimap<Gamepad::Axis, std::tuple<std::string, float>, EnumClassHash> m_GamepadAxisBindings; // Gamepad::Axis -> command string & value
	//std::unordered_multimap<Gamepad::Button, std::tuple<std::string, float>, EnumClassHash> m_GamepadButtonBindings; // Gamepad::Button -> command string

	//// Input events
	//EventRelay<InputProxy, Events::MousePress> m_EMousePress;
	//bool OnMousePress(const Events::MousePress &event);
	//EventRelay<InputProxy, Events::MouseRelease> m_EMouseRelease;
	//bool OnMouseRelease(const Events::MouseRelease &event);
	//EventRelay<InputProxy, Events::GamepadAxis> m_EGamepadAxis;
	//bool OnGamepadAxis(const Events::GamepadAxis &event);
	//EventRelay<InputProxy, Events::GamepadButtonDown> m_EGamepadButtonDown;
	//bool OnGamepadButtonDown(const Events::GamepadButtonDown &event);
	//EventRelay<InputProxy, Events::GamepadButtonUp> m_EGamepadButtonUp;
	//bool OnGamepadButtonUp(const Events::GamepadButtonUp &event);
	//// Input binding events
	//EventRelay<InputProxy, Events::BindMouseButton> m_EBindMouseButton;
	//bool OnBindMouseButton(const Events::BindMouseButton &event);
	//EventRelay<InputProxy, Events::BindGamepadAxis> m_EBindGamepadAxis;
	//bool OnBindGamepadAxis(const Events::BindGamepadAxis &event);
	//EventRelay<InputProxy, Events::BindGamepadButton> m_EBindGamepadButton;
	//bool OnBindGamepadButton(const Events::BindGamepadButton &event);

	//float GetCommandTotalValue(std::string command);
	//void PublishCommand(int playerID, std::string command, float value);
};

#endif
