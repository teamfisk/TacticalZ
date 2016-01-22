#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "Systems/SpawnerSystem.h"
#include "Events/ESpawnerSpawn.h"

class PlayerSpawnSystem : public ImpureSystem
{
public:
    PlayerSpawnSystem(World* world, EventBroker* eventBroker);

    virtual void Update(double dt) override;

private:
    EventRelay<PlayerSpawnSystem, Events::InputCommand> m_OnInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);

    std::vector<int> m_SpawnRequests;
};