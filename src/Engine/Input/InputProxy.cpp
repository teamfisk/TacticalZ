#include "Input/InputProxy.h"
#include "Input/InputHandler.h"

InputProxy::InputProxy(EventBroker* eventBroker) 
    : m_EventBroker(eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_EBindOrigin, &InputProxy::OnBindOrigin);
}

InputProxy::~InputProxy()
{
    for (auto& handler : m_Handlers) {
        delete handler;
    }
}

void InputProxy::LoadBindings(std::string file)
{
    auto config = ResourceManager::Load<ConfigFile>(file);
    for (auto& origin : config->GetAll<std::string>("Bindings")) {
        Events::BindOrigin e;
        e.Origin = origin.first;
        e.Command = origin.second;
        e.Value = 1.f;
        if (!e.Command.empty()) {
            char prefix = e.Command.at(0);
            if (prefix == '+' || prefix == '-') {
                e.Command = e.Command.substr(1);
                if (prefix == '-') {
                    e.Value *= -1.f;
                }
            }
            OnBindOrigin(e);
        }
    }
}

void InputProxy::Update(double dt)
{
    m_EventBroker->Process<InputProxy>();
    m_EventBroker->Process<InputHandler>();
    for (auto& handler : m_Handlers) {
        handler->Update(dt);
    }
}

void InputProxy::Process()
{
    for (auto& pair : m_CommandHandlers) {
        const std::string& command = pair.first;
        auto handlers = pair.second;
        m_CurrentCommandValues[command] = 0.f;
        for (auto& handler : handlers) {
            m_CurrentCommandValues[command] += handler->GetCommandValue(command);
        }

        auto last = m_LastCommandValues.find(command);
        float currentValue = m_CurrentCommandValues[command];
        if (last == m_LastCommandValues.end() || last->second != currentValue) {
            Events::InputCommand e;
            e.PlayerID = -1;
            e.Command = command;
            e.Value = currentValue;
            m_EventBroker->Publish(e);
            LOG_DEBUG("Input: Published command %s=%f for player %i", e.Command.c_str(), e.Value, e.PlayerID);
            m_LastCommandValues[command] = currentValue;
        }
    }
}

bool InputProxy::OnBindOrigin(const Events::BindOrigin& e)
{
    bool originBound = false;
    for (auto& handler : m_Handlers) {
        bool result = handler->BindOrigin(e.Origin, e.Command, e.Value);
        if (result) {
            m_CommandHandlers[e.Command].insert(handler);
            m_LastCommandValues[e.Command] = 0.f;
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
