#ifndef EVENTFIXTURE_H
#define EVENTFIXTURE_H

#include <boost/test/unit_test.hpp>
#include "Core/EventBroker.h"

template <typename EventType>
struct EventFixture
{
    EventFixture()
    {
        this->ventBroker = new EventBroker();
        m_EEventType = decltype(m_EEventType)(std::bind(&EventFixture::OnEvent, this, std::placeholders::_1));
        this->ventBroker->Subscribe(m_EEventType);
        Run();
        Check();
    }
    ~EventFixture()
    {
        this->ventBroker->Unsubscribe(m_EEventType);
        delete this->ventBroker;
    }

    EventBroker* ventBroker = nullptr;
    EventRelay<EventFixture, EventType> m_EEventType;
    bool m_EventRecieved = false;
    EventType Before;
    EventType After;

    bool OnEvent(const EventType& event)
    {
        m_EventRecieved = true;
        After = event;

        return true;
    }

    void Run()
    {
        // Publish the event
        this->ventBroker->Publish(Before);
        // Clear to swap buffers
        this->ventBroker->Swap();
        // Process the event
        this->ventBroker->template Process<EventFixture>();
    }

    void Check()
    {
        BOOST_CHECK(m_EventRecieved);
    }
};

#endif