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
    bool OnPlayerDeath(const Events::PlayerDeath& e);
    EventRelay<ScoreScreenSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawn(const Events::PlayerSpawned& e);

private:
    struct PlayerData {
        int ID = -1;
        std::string Name = "";
        int Team = -1;
        EntityWrapper Player = EntityWrapper::Invalid;
    };

    int m_PlayerCounter = 0;
    std::unordered_map<std::string, PlayerData> m_PlayerIdentities;
};

#endif