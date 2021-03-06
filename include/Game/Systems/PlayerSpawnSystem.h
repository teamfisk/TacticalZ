#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "Systems/SpawnerSystem.h"
#include "Events/ESpawnerSpawn.h"
#include "Core/EPlayerSpawned.h"
#include "Core/EPlayerDeath.h"
#include "Rendering/ESetCamera.h"
#include "Core/ConfigFile.h"

class PlayerSpawnSystem : public ImpureSystem
{
public:
    PlayerSpawnSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    // This enum must correspond to the command values for PickTeam buttons.
    enum class PlayerClass
    {
        None = 0,
        Assault = 1,
        Defender = 2,
        Sniper = 2
    };
    struct SpawnRequest
    {
        int PlayerID;
        ComponentInfo::EnumType Team;
        PlayerClass Class;
    };

    bool m_NetworkEnabled = false;
    std::vector<SpawnRequest> m_SpawnRequests;

    //Player ID -> EntityWrapper.
    std::map<int, EntityWrapper> m_PlayerEntities;
    //EntityWrapper ID -> Player ID.
    std::map<EntityID, int> m_PlayerIDs;

    float m_ForcedRespawnTime;
    bool m_DbgConfigForceRespawn;

    EventRelay<PlayerSpawnSystem, Events::InputCommand> m_OnInputCommand;
    bool OnInputCommand(Events::InputCommand& e);
    EventRelay<PlayerSpawnSystem, Events::PlayerSpawned> m_OnPlayerSpawnerd;
    bool OnPlayerSpawned(Events::PlayerSpawned& e);
    EventRelay<PlayerSpawnSystem, Events::PlayerDeath> m_OnPlayerDeath;
    bool OnPlayerDeath(Events::PlayerDeath& e);
};