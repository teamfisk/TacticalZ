#ifndef RespawnSystem_h__
#define RespawnSystem_h__

#include "System.h"
#include "SpawnerSystem.h"
#include "EEntityDeleted.h"

class RespawnSystem : public PureSystem
{
public:
    RespawnSystem(SystemParams params);

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cRespawn, double dt) override;

private:
    std::unordered_map<EntityWrapper, EntityWrapper> m_LastRespawnedEntity;

    EventRelay<RespawnSystem, Events::EntityDeleted> m_EEntityDeleted;
    bool OnEntityDeleted(const Events::EntityDeleted& e);
};

#endif