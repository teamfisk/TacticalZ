#ifndef InputHandler_h__
#define InputHandler_h__

#include "../Common.h"
#include "../Core/EventBroker.h"
#include "InputProxy.h"

class InputHandler
{
public:
    InputHandler(EventBroker* eventBroker, InputProxy* inputProxy)
        : m_EventBroker(eventBroker)
        , m_InputProxy(inputProxy)
    { }

    virtual bool BindOrigin(std::string origin, std::string command, float value) = 0;
    virtual void Update(double dt) { }
    virtual float GetCommandValue(std::string command) = 0;

protected:
    EventBroker* m_EventBroker;
    InputProxy* m_InputProxy;
};

#endif