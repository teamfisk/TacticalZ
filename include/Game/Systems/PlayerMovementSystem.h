#include "Common.h"
#include "GLM.h"
#include "Core/System.h"
#include "Core/EPlayerSpawned.h"
#include "Input/FirstPersonInputController.h"
#include <imgui/imgui.h>

class PlayerMovementSystem : public ImpureSystem
{
public:
    PlayerMovementSystem(SystemParams params);
    ~PlayerMovementSystem();

    virtual void Update(double dt) override;

private:
    // State
    std::unordered_map<EntityWrapper, FirstPersonInputController<PlayerMovementSystem>*> m_PlayerInputControllers;

    EventRelay<PlayerMovementSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(Events::PlayerSpawned& e);

    void updateMovementControllers(double dt);
    void updateVelocity(double dt);
};