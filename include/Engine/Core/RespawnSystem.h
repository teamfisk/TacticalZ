#ifndef RespawnSystem_h__
#define RespawnSystem_h__

#include "System.h"
#include "SpawnerSystem.h"
#include "EEntityDeleted.h"
#include "EPause.h"

class RespawnSystem : public PureSystem
{
public:
    RespawnSystem(SystemParams params);

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cRespawn, double dt) override;

private:
    std::unordered_map<EntityWrapper, EntityWrapper> m_LastRespawnedEntity;
    bool m_Paused = false;

    EventRelay<RespawnSystem, Events::EntityDeleted> m_EEntityDeleted;
    bool OnEntityDeleted(const Events::EntityDeleted& e);
    EventRelay<RespawnSystem, Events::Pause> m_EPause;
    bool OnPause(const Events::Pause& e);
    EventRelay<RespawnSystem, Events::Resume> m_EResume;
    bool OnResume(const Events::Resume& e);
};

#endif