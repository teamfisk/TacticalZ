#ifndef Systems_SoundSystem_h__
#define Systems_SoundSystem_h__

#include "../Engine/Core/System.h"
#include "../Engine/Sound/SoundManager.h"
#include "../Engine/Core/EPlayerSpawned.h"
#include "../Engine/Input/EInputCommand.h"


class SoundSystem : public PureSystem, ImpureSystem, SoundManager
{
public:
    SoundSystem(World* world, EventBroker* eventbroker);
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt) override;
    virtual void Update(double dt) override;
private:
    // The logic for making the sound play when player is moving
    void playerStep(double dt);
    EntityWrapper m_LocalPlayer = EntityWrapper();

    World* m_World = nullptr;
    EventBroker* m_EventBroker = nullptr;

    // Logic for playing a sound when a player jumps
    void playerJumps();

    // Walking logic
    // Keeps track of how far the player has walked within this "key press session".
    float m_DistanceMoved = 0.0f;
    // How far a step is (How often the step sound will be played).
    const float m_PlayerStepLength = 1.75f;
    // To get a difference when calculating the walking state.
    glm::vec3 m_LastPosition = glm::vec3();
    // Determine what sound file to play.
    bool m_LeftFoot = false;

    EventRelay<SoundSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(const Events::PlayerSpawned &e);
    EventRelay<SoundSystem, Events::InputCommand> m_InputCommand;
    bool OnInputCommand(const Events::InputCommand &e);
};

#endif
