#include "Common.h"
#include "GLM.h"
#include "Core/System.h"
#include "Core/EPlayerSpawned.h"
#include "Input/FirstPersonInputController.h"
#include <imgui/imgui.h>
#include "Events/EDoubleJump.h"
#include "../Engine/Sound/EPlaySoundOnEntity.h"

#include "Core/EntityFile.h"
#include "Core/EntityFileParser.h"

class PlayerMovementSystem : public ImpureSystem
{
public:
    PlayerMovementSystem(SystemParams params);
    ~PlayerMovementSystem();

    virtual void Update(double dt) override;

private:
    // State
    std::unordered_map<EntityWrapper, FirstPersonInputController<PlayerMovementSystem>*> m_PlayerInputControllers;

    EntityWrapper m_LocalPlayer = EntityWrapper::Invalid;
    // Walking logic
    // Keeps track of how far the player has walked within this "key press session".
    float m_DistanceMoved = 0.0f;
    // How far a step is (How often the step sound will be played).
    const float m_PlayerStepLength = 1.75f;
    // Determine what sound file to play.
    bool m_LeftFoot = false;
    // To get a difference when calculating the walking state.
    glm::vec3 m_LastPosition = glm::vec3();
    // The logic for making the sound play when player is moving
    void playerStep(double dt);
    // Spawn a hexagon at origin of an Entity
    void spawnHexagon(EntityWrapper target);

    EventRelay<PlayerMovementSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(Events::PlayerSpawned& e);
    EventRelay<PlayerMovementSystem, Events::DoubleJump> m_EDoubleJump;
    bool PlayerMovementSystem::OnDoubleJump(Events::DoubleJump & e);

    void updateMovementControllers(double dt);
    void updateVelocity(EntityWrapper player, double dt);
};