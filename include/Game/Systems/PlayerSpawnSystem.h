#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "Systems/SpawnerSystem.h"
#include "Events/ESpawnerSpawn.h"
#include "Events/EPlayerSpawned.h"
#include "Rendering/ESetCamera.h"

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

    EventRelay<PlayerSpawnSystem, Events::InputCommand> m_OnInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);

    std::vector<SpawnRequest> m_SpawnRequests;
};