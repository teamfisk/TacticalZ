#ifndef Systems_SoundSystem_h__
#define Systems_SoundSystem_h__

#include <random>

#include "../Engine/Core/System.h"
#include "../Engine/Core/ResourceManager.h"
#include "../Engine/Sound/Sound.h"
#include "../Engine/Sound/EPlayQueueOnEntity.h"
#include "../Engine/Core/EPlayerSpawned.h"
#include "../Engine/Input/EInputCommand.h"
#include "../Engine/Core/EShoot.h"
#include "../Engine/Core/EPlayerSpawned.h"
#include "../Engine/Input/EInputCommand.h"
#include "../Engine/Core/ECaptured.h"
#include "../Engine/Core/EPlayerDamage.h"
#include "../Engine/Core/EPlayerDeath.h"
#include "../Engine/Core/EPlayerHealthPickup.h"
#include "../Engine/Collision/ETrigger.h"
#include "../Engine/Sound/EPlaySoundOnEntity.h"
#include "../Engine/Sound/EPlayBackgroundMusic.h"
#include "../Game/Events/EDoubleJump.h"
#include "../Game/Events/EDashAbility.h"


class SoundSystem : public PureSystem, ImpureSystem
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
    // Helper function
    EntityID createChildEmitter(EntityWrapper parent);

    // Walking logic
    // Keeps track of how far the player has walked within this "key press session".
    float m_DistanceMoved = 0.0f;
    // How far a step is (How often the step sound will be played).
    const float m_PlayerStepLength = 1.75f;
    // To get a difference when calculating the walking state.
    glm::vec3 m_LastPosition = glm::vec3();
    // Determine what sound file to play.
    bool m_LeftFoot = false;

    std::default_random_engine generator;

    EventRelay<SoundSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(const Events::PlayerSpawned &e);
    EventRelay<SoundSystem, Events::InputCommand> m_InputCommand;
    bool OnInputCommand(const Events::InputCommand &e);
    EventRelay<SoundSystem, Events::DoubleJump> m_EDoubleJump;
    bool OnDoubleJump(const Events::DoubleJump &e);
    EventRelay<SoundSystem, Events::DashAbility> m_EDashAbility;
    bool OnDashAbility(const Events::DashAbility &e);
    EventRelay<SoundSystem, Events::TriggerTouch> m_ETriggerTouch;
    bool OnTriggerTouch(const Events::TriggerTouch &e);
    EventRelay<SoundSystem, Events::Shoot> m_EShoot;
    bool OnShoot(const Events::Shoot &e);
    EventRelay<SoundSystem, Events::Captured> m_ECaptured;
    bool OnCaptured(const Events::Captured &e);
    EventRelay<SoundSystem, Events::PlayerDamage> m_EPlayerDamage;
    bool OnPlayerDamage(const Events::PlayerDamage &e);
    EventRelay<SoundSystem, Events::PlayerDeath> m_EPlayerDeath;
    bool OnPlayerDeath(const Events::PlayerDeath &e);
    EventRelay<SoundSystem, Events::PlayerHealthPickup> m_EPlayerHealthPickup;
    bool OnPlayerHealthPickup(const Events::PlayerHealthPickup &e);
};

#endif
