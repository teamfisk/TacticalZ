#include "Systems/LifetimeSystem.h"

void LifetimeSystem::Update(double dt)
{
    for (auto& e : m_Deletions) {
        m_World->DeleteEntity(e.ID);
    }
    m_Deletions.clear();
}

void LifetimeSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cLifetime, double dt)
{
    if (dt == 0.0) {
        return;
    }

    if (!entity.Valid()) {
        return;
    }

    double& lifetime = cLifetime["Lifetime"];
    lifetime -= dt;

    if (lifetime <= 0.0) {
        m_Deletions.push_back(entity);
    }
}