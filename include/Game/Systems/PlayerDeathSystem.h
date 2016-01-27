#ifndef PlayerDeathSystem_h__
#define PlayerDeathSystem_h__

#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "Systems/SpawnerSystem.h"
#include "Events/ESpawnerSpawn.h"
#include "Core/EPlayerSpawned.h"
#include "Rendering/ESetCamera.h"
#include "Core/ConfigFile.h"

#include "Core/EPlayerDeath.h"
//tests
#include "Core/EPlayerDamage.h"

class PlayerDeathSystem : public ImpureSystem
{
public:
    PlayerDeathSystem(World* world, EventBroker* eventBroker);

    virtual void Update(double dt) override;

private:
    struct SpawnRequest
    {
        int PlayerID;
        ComponentInfo::EnumType Team;
    };

    bool m_NetworkEnabled = false;
    std::vector<SpawnRequest> m_SpawnRequests;
    std::map<int, EntityWrapper> m_PlayerEntities;

    EventRelay<PlayerDeathSystem, Events::InputCommand> m_OnInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<PlayerDeathSystem, Events::PlayerDeath> m_OnPlayerDeath;
    bool OnPlayerDeath(Events::PlayerDeath& e);
};
#endif