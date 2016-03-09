#include "Core/RespawnSystem.h"

RespawnSystem::RespawnSystem(SystemParams params) 
    : System(params)
    , PureSystem("Respawn")
{
    EVENT_SUBSCRIBE_MEMBER(m_EEntityDeleted, &RespawnSystem::OnEntityDeleted);
    EVENT_SUBSCRIBE_MEMBER(m_EPause, &RespawnSystem::OnPause);
    EVENT_SUBSCRIBE_MEMBER(m_EResume, &RespawnSystem::OnResume);
}

void RespawnSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cRespawn, double dt)
{
    if (m_Paused) {
        return;
    }

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

bool RespawnSystem::OnPause(const Events::Pause& e)
{
    if (e.World == m_World) {
        m_Paused = true;
    }
    return true;
}

bool RespawnSystem::OnResume(const Events::Resume& e)
{
    if (e.World == m_World) {
        m_Paused = false;
    }
    return true;
}

