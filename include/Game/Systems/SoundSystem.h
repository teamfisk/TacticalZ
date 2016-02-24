#ifndef Systems_SoundSystem_h__
#define Systems_SoundSystem_h__

#include <random>

#include "../Engine/Core/System.h"
#include "../Engine/Core/ResourceManager.h"
#include "../Engine/Core/ConfigFile.h"
#include "../Engine/Sound/Sound.h"
#include "../Engine/Sound/EPlayQueueOnEntity.h"
#include "../Engine/Core/EPlayerSpawned.h"
#include "../Engine/Input/EInputCommand.h"
#include "../Engine/Core/EShoot.h"
#include "../Engine/Core/EPlayerSpawned.h"
#include "../Engine/Input/EInputCommand.h"
#include "Events/ECaptured.h"
#include "../Engine/Core/EPlayerDamage.h"
#include "../Engine/Core/EPlayerDeath.h"
#include "Events/EPlayerHealthPickup.h"
#include "../Engine/Collision/ETrigger.h"
#include "../Engine/Sound/EPlaySoundOnEntity.h"
#include "../Engine/Sound/EPlayBackgroundMusic.h"
#include "../Game/Events/EDoubleJump.h"
#include "../Game/Events/EDashAbility.h"


class SoundSystem : public PureSystem, ImpureSystem
{
public:
    SoundSystem(SystemParams params);
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt) override;
    virtual void Update(double dt) override;
private:
    std::string m_Announcer = "";
    // Logic for playing a sound when a player jumps
    void playerJumps();
    
    // Temporary solution for play test.
    bool m_DrumsIsPlaying = false;
    double m_DrumTimer = 0.0;
    bool drumTimer(double dt);

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
