#ifndef ScoreScreenSystem_h__
#define ScoreScreenSystem_h__

#include "Core/System.h"
#include "Core/EPlayerDeath.h"
#include "Core/EPlayerSpawned.h"
#include "GLM.h"

class ScoreScreenSystem : public PureSystem
{
public:
    ScoreScreenSystem(SystemParams params);

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& capturePoint, double dt) override;

    EventRelay<ScoreScreenSystem, Events::PlayerDeath> m_EPlayerDeath;
    void OnPlayerDeath(const Events::PlayerDeath& e);
    EventRelay<ScoreScreenSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    void OnPlayerSpawn(const Events::PlayerSpawned& e);
};

#endif