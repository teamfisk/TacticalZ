#include "Network/MultiplayerSnapshotFilter.h"

MultiplayerSnapshotFilter::MultiplayerSnapshotFilter(EventBroker* eventBroker) 
    : m_EventBroker(eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &MultiplayerSnapshotFilter::OnPlayerSpawned);
}

bool MultiplayerSnapshotFilter::FilterComponent(EntityWrapper entity, SharedComponentWrapper& component)
{
    if (entity == m_LocalPlayer || entity.IsChildOf(m_LocalPlayer)) {
        return false;
    }

    if (component.Info.Name == "Physics") {
        return false;
    }

    if (component.Info.Name == "Transform" || component.Info.Name == "Physics") {
        m_EventBroker->Publish(Events::Interpolate(entity, component));
        return false;
    }

    return true;
}

bool MultiplayerSnapshotFilter::OnPlayerSpawned(Events::PlayerSpawned ePlayerSpawned)
{
    m_LocalPlayer = ePlayerSpawned.Player;
    return true;
}