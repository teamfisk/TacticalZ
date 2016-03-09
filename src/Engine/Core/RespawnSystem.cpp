#include "Core/RespawnSystem.h"

RespawnSystem::RespawnSystem(SystemParams params) 
    : System(params)
    , PureSystem("Respawn")
{
    EVENT_SUBSCRIBE_MEMBER(m_EEntityDeleted, &RespawnSystem::OnEntityDeleted);
}

void RespawnSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cRespawn, double dt)
{
    if (!entity.HasComponent("Spawner")) {
        return;
    }

    EntityWrapper& lastRespawnedEntity = m_LastRespawnedEntity[entity];
    if (!lastRespawnedEntity.Valid()) {
        double& _respawnCountdown = cRespawn["_RespawnCountdown"];
        _respawnCountdown -= dt;
        if (_respawnCountdown <= 0) {
            lastRespawnedEntity = SpawnerSystem::Spawn(entity, entity);
            _respawnCountdown = cRespawn["Delay"];
        }
    }
}

bool RespawnSystem::OnEntityDeleted(const Events::EntityDeleted& e)
{
    m_LastRespawnedEntity.erase(e.DeletedEntity);
    return true;
}

