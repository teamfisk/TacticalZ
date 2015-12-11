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
        OnBindOrigin(e);
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

void InputProxy::Publish(const Events::InputCommand& e)
{
    auto key = std::make_pair(e.PlayerID, e.Command);
    m_CommandQueue[key].push_back(e.Value);
}

bool InputProxy::OnBindOrigin(const Events::BindOrigin& e)
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
