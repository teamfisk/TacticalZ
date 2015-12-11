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
	InputController(std::shared_ptr<dd::EventBroker> eventBroker)
		: EventBroker(eventBroker)
	{ Initialize(); }

	virtual void Initialize()
	{
		EVENT_SUBSCRIBE_MEMBER(
            _EInputCommand, &InputController::OnCommand);
		EVENT_SUBSCRIBE_MEMBER(m_EMouseMove, &InputController::OnMouseMove);
	}

	virtual bool OnCommand(const Events::InputCommand &event) { return false; }
	virtual bool OnMouseMove(const Events::MouseMove &event) { return false; }

protected:
	std::shared_ptr<dd::EventBroker> EventBroker;

private:
	EventRelay<EventContext, Events::InputCommand> m_EInputCommand;
	EventRelay<EventContext, Events::MouseMove> m_EMouseMove;
};

#endif
