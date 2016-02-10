#include "Common.h"
#include "GLM.h"
#include "Core/System.h"
#include "Core/EPlayerSpawned.h"
#include "Input/FirstPersonInputController.h"
#include <imgui/imgui.h>

#include "Core/EntityFile.h"
#include "Core/EntityFileParser.h"

class PlayerMovementSystem : public ImpureSystem, PureSystem
{
public:
    PlayerMovementSystem(World* world, EventBroker* eventBroker);
    ~PlayerMovementSystem();

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt);

private:
    // State
    std::unordered_map<EntityWrapper, FirstPersonInputController<PlayerMovementSystem>*> m_PlayerInputControllers;

    EventRelay<PlayerMovementSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(Events::PlayerSpawned& e);

};