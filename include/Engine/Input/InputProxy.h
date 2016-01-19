#ifndef InputProxy_h__
#define InputProxy_h__

#include <boost/tokenizer.hpp>
#include "../Common.h"
#include "../Core/ResourceManager.h"
#include "../Core/ConfigFile.h"
#include "EInputCommand.h"
#include "EBindOrigin.h"

class InputHandler;

class InputProxy
{
public:
    InputProxy(EventBroker* eventBroker);
    ~InputProxy();
    
    void LoadBindings(std::string file);
	void Update(double dt);
    void Process();
    template <typename T>
    void AddHandler();
    void Publish(const Events::InputCommand& e);

protected:
    EventBroker* m_EventBroker;
    std::vector<InputHandler*> m_Handlers;
    std::map<std::string, std::set<InputHandler*>> m_CommandHandlers;
    
    // Represents every unique command (has of PlayerID & Command) and all values reported for that command this frame
    std::map<std::pair<unsigned int, std::string>, std::vector<float>> m_CommandQueue;

    std::map<std::string, float> m_CurrentCommandValues;
    std::map<std::string, float> m_LastCommandValues;

	EventRelay<InputProxy, Events::BindOrigin> m_EBindOrigin;
	bool OnBindOrigin(const Events::BindOrigin& e);
};

template <typename T>
void InputProxy::AddHandler()
{
    m_Handlers.push_back(new T(m_EventBroker, this));
}

#endif
