#ifndef ScoreScreenSystem_h__
#define ScoreScreenSystem_h__

#include "Core/System.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFile.h"
#include "Network/EKillDeath.h"
#include "Core/EPlayerSpawned.h"
#include "Network/EPlayerConnected.h"
#include "Network/EPlayerDisconnected.h"
#include "GLM.h"

class ScoreScreenSystem : public PureSystem
{
public:
    ScoreScreenSystem(SystemParams params);

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& capturePoint, double dt) override;

    EventRelay<ScoreScreenSystem, Events::KillDeath> m_EPlayerDeath;
    bool OnPlayerDeath(const Events::KillDeath& e);
    EventRelay<ScoreScreenSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawn(const Events::PlayerSpawned& e);
    EventRelay<ScoreScreenSystem, Events::PlayerConnected> m_EPlayerConnected;
    bool OnPlayerConnected(const Events::PlayerConnected& e);
    EventRelay<ScoreScreenSystem, Events::PlayerDisconnected> m_EPlayerDisconnected;
    bool OnPlayerDisconnected(const Events::PlayerDisconnected& e);

private:
    struct PlayerData {
        int ID = -1;
        std::string Name = "";
        int Team = 1;
        int Kills = 0;
        int Deaths = 0;
        EntityWrapper Player = EntityWrapper::Invalid;
    };

    std::vector<int> m_DisconnectedIdentities;
    int m_PlayerCounter = 0;
    std::unordered_map<int, PlayerData> m_PlayerIdentities;
};

#endif