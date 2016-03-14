#include "Network/MultiplayerSnapshotFilter.h"

MultiplayerSnapshotFilter::MultiplayerSnapshotFilter(EventBroker* eventBroker) 
    : m_EventBroker(eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &MultiplayerSnapshotFilter::OnPlayerSpawned);
}

bool MultiplayerSnapshotFilter::FilterComponent(EntityWrapper entity, SharedComponentWrapper& component)
{
    if (entity == m_LocalPlayer || entity.IsChildOf(m_LocalPlayer)) {
        if (
            component.Info.Name == "Transform"
            || component.Info.Name == "Physics"
            || component.Info.Name == "AssaultWeapon"
            || component.Info.Name == "DefenderWeapon"
            || component.Info.Name == "SidearmWeapon"
            || component.Info.Name == "Animation"
            || component.Info.Name == "Blend"
            || component.Info.Name == "BlendAdditive"
            || component.Info.Name == "BlendOverride"
            || entity.Name() == "PlayerName"
        ) {
            return false;
        }
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
    if (ePlayerSpawned.PlayerID == -1) {
        m_LocalPlayer = ePlayerSpawned.Player;
    }
    return true;
}