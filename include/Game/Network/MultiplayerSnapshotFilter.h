#ifndef MultiplayerSnapshotFilter_h__
#define MultiplayerSnapshotFilter_h__

#include "Core/EventBroker.h"
#include "Core/EPlayerSpawned.h"
#include "Network/SnapshotFilter.h"
#include "Network/EInterpolate.h"

class MultiplayerSnapshotFilter : public SnapshotFilter
{
public:
    MultiplayerSnapshotFilter(EventBroker* eventBroker);

    virtual bool FilterComponent(EntityWrapper entity, SharedComponentWrapper& component) override;

private:
    EventBroker* m_EventBroker;

    EntityWrapper m_LocalPlayer = EntityWrapper::Invalid;

    EventRelay<MultiplayerSnapshotFilter, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(Events::PlayerSpawned ePlayerSpawned);
};

#endif