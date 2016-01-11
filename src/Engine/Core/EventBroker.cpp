#include "Core/EventBroker.h"

BaseEventRelay::~BaseEventRelay()
{
    if (m_Broker != nullptr) {
		m_Broker->Unsubscribe(*this);
    }
}

void EventBroker::Unsubscribe(BaseEventRelay& relay) // ?
{
    auto identifier = std::make_tuple(relay.m_EventID, relay.m_ContextTypeName, relay.m_EventTypeName);

    relay.m_Broker = nullptr;
    if (m_IsProcessing) {
        m_RelaysToUnsubscribe.push_back(identifier);
    } else {
        unsubscribeImmediate(identifier);
    }
}

void EventBroker::Subscribe(BaseEventRelay& relay)
{
	if (m_IsProcessing) {
		m_RelaysToSubscribe.push_back(&relay);
	} else {
		subscribeImmediate(relay);
	}
}

int EventBroker::Process(std::string contextTypeName)
{
	m_IsProcessing = true;

	auto it = m_ContextRelays.find(contextTypeName);
	if (it == m_ContextRelays.end()) {
		return 0;
	}

	EventRelays_t &relays = it->second;

	int eventsProcessed = 0;
	for (auto &pair : *m_EventQueueRead) {
		std::string& eventTypeName = pair.first;
		std::shared_ptr<Event> event = pair.second;

		auto itpair = relays.equal_range(eventTypeName);
		for (auto it2 = itpair.first; it2 != itpair.second; it2++) {
			std::string name = it2->first;
			BaseEventRelay* relay = it2->second;
			relay->Receive(event);
			eventsProcessed++;
		}
	}

	m_IsProcessing = false;

	// Process pending subscriptions
	for (auto& r : m_RelaysToSubscribe) {
		subscribeImmediate(*r);
	}
	m_RelaysToSubscribe.clear();

	// Process pending unsubscriptions
	for (auto& identifier : m_RelaysToUnsubscribe) {
		unsubscribeImmediate(identifier);
	}
	m_RelaysToUnsubscribe.clear();

	return eventsProcessed;
}

void EventBroker::Swap()
{
	std::swap(m_EventQueueRead, m_EventQueueWrite);
}

void EventBroker::Clear()
{
	m_EventQueueWrite->clear();
}

void EventBroker::subscribeImmediate(BaseEventRelay& relay)
{
	relay.m_Broker = this;
    relay.m_EventID = m_NextEventID++;
	m_ContextRelays[relay.m_ContextTypeName].insert(std::make_pair(relay.m_EventTypeName, &relay));
}

void EventBroker::unsubscribeImmediate(std::tuple<EventID, ContextTypeName_t, EventTypeName_t> identifier)
{
    EventID eventID;
    ContextTypeName_t contextTypeName;
    EventTypeName_t eventTypeName;
    std::tie(eventID, contextTypeName, eventTypeName) = identifier;

	auto contextIt = m_ContextRelays.find(contextTypeName);
	if (contextIt == m_ContextRelays.end()) {
		return;
	}

	auto eventRelays = contextIt->second;
	auto itpair = eventRelays.equal_range(eventTypeName);
	for (auto it = itpair.first; it != itpair.second; ++it) {
		if (it->second->m_EventID == eventID) {
			eventRelays.erase(it);
			break;
		}
	}
}
