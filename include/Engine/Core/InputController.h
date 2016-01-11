#ifndef InputController_h__
#define InputController_h__

#include "../Common.h"

#include "EventBroker.h"
#include "EMouseMove.h"
#include "../Input/EInputCommand.h"

template <typename EventContext>
class InputController
{
public:
	InputController(EventBroker* eventBroker)
		: m_EventBroker(eventBroker)
	{ Initialize(); }

	virtual void Initialize()
	{
		EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &InputController::OnCommand);
	}

	virtual bool OnCommand(const Events::InputCommand& e) { return false; }

protected:
	EventBroker* m_EventBroker;

private:
	EventRelay<EventContext, Events::InputCommand> m_EInputCommand;
};

#endif
