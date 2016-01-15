#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "Events/ESpawnerSpawn.h"

class PlayerSpawnSystem : public ImpureSystem
{
public:
    PlayerSpawnSystem(EventBroker* eventBroker);

    virtual void Update(World* world, double dt) override;

private:
    EventRelay<PlayerSpawnSystem, Events::InputCommand> m_OnInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);

    std::vector<int> m_SpawnRequests;
};