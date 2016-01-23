#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "Systems/SpawnerSystem.h"
#include "Events/ESpawnerSpawn.h"
#include "Events/EPlayerSpawned.h"
#include "Rendering/ESetCamera.h"
#include "Core/ConfigFile.h"

class PlayerSpawnSystem : public ImpureSystem
{
public:
    PlayerSpawnSystem(World* world, EventBroker* eventBroker);

    virtual void Update(double dt) override;

private:
    struct SpawnRequest
    {
        int PlayerID;
        ComponentInfo::EnumType Team;
    };

    bool m_NetworkEnabled = false;
    std::vector<SpawnRequest> m_SpawnRequests;

    EventRelay<PlayerSpawnSystem, Events::InputCommand> m_OnInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<PlayerSpawnSystem, Events::PlayerSpawned> m_OnPlayerSpawnerd;
    bool OnPlayerSpawned(Events::PlayerSpawned& e);
};