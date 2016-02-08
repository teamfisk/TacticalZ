#include "Game/Systems/SoundSystem.h"

SoundSystem::SoundSystem(World* world, EventBroker* eventbroker)
    : System(world, eventbroker)
    , PureSystem("SoundEmitter")
    , ImpureSystem()
{
    m_World = world;
    m_EventBroker = eventbroker;
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &SoundSystem::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_InputCommand, &SoundSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EDoubleJump, &SoundSystem::OnDoubleJump);
    EVENT_SUBSCRIBE_MEMBER(m_EDashAbility, &SoundSystem::OnDashAbility);
    EVENT_SUBSCRIBE_MEMBER(m_EShoot, &SoundSystem::OnShoot);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &SoundSystem::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &SoundSystem::OnPlayerDamage);
    EVENT_SUBSCRIBE_MEMBER(m_ECaptured, &SoundSystem::OnCaptured);
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &SoundSystem::OnTriggerTouch);
}

void SoundSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt)
{

}

void SoundSystem::Update(double dt)
{
}

bool SoundSystem::OnPlayerSpawned(const Events::PlayerSpawned &e)
{
    if (e.PlayerID == -1) { // Local player
        m_World->AttachComponent(e.Player.ID, "Listener");
        m_LocalPlayer = e.Player;
        Events::PlaySoundOnEntity event;
        event.EmitterID = createChildEmitter(m_LocalPlayer);
        event.FilePath = "Audio/announcer/go.wav";
        m_EventBroker->Publish(event);
        // TEMP: starts bgm
        {
            Events::PlayBackgroundMusic ev;
            ev.FilePath = "Audio/bgm/ambient.wav";
            m_EventBroker->Publish(ev);
        }
    }
    return true;
}

bool SoundSystem::OnInputCommand(const Events::InputCommand & e)
{
    if (e.Command == "Jump" && e.Value > 0) {
        if (e.PlayerID == -1) { // local player
            playerJumps();
            return true;
        }
    }
    if (e.Command == "Forward" || e.Command == "Right") {
        if (e.Value == 0) {
            // Key released
            // Reset the distance moved (player walk logic)
            m_DistanceMoved = 0.f;
        }
    }
    if (e.Command == "TakeDamage" && e.Value > 0) {
        Events::PlayerDamage ev;
        ev.Player = m_LocalPlayer;
        ev.Damage = 1.0;
        m_EventBroker->Publish(ev);
    }

    return false;
}

void SoundSystem::playerJumps()
{
    glm::vec3 vel = (glm::vec3)m_World->GetComponent(m_LocalPlayer.ID, "Physics")["Velocity"];
    if (vel.y == 0) {
        Events::PlaySoundOnEntity e;
        e.EmitterID = createChildEmitter(m_LocalPlayer);
        e.FilePath = "Audio/jump/jump1.wav";
        m_EventBroker->Publish(e);
    }
}

bool SoundSystem::OnShoot(const Events::Shoot & e)
{
    Events::PlaySoundOnEntity ev;
    ev.EmitterID = createChildEmitter(m_LocalPlayer);
    ev.FilePath = "Audio/laser/laser1.wav";
    m_EventBroker->Publish(ev);
    return true;
}

bool SoundSystem::OnCaptured(const Events::Captured & e)
{
    int homeTeam = (int)m_World->GetComponent(e.CapturePointID, "Team")["Team"];
    int team = (int)m_World->GetComponent(m_LocalPlayer.ID, "Team")["Team"];
    Events::PlaySoundOnEntity ev;
    if (team == homeTeam) {
        ev.FilePath = "Audio/announcer/objective_achieved.wav";
    } else {
        ev.FilePath = "Audio/announcer/objective_failed.wav"; // have not been tested
    }
    ev.EmitterID = createChildEmitter(m_LocalPlayer);
    m_EventBroker->Publish(ev);
    return false;
}

// Testing purposes atm...
bool SoundSystem::OnPlayerDamage(const Events::PlayerDamage & e)
{
    // Should check for only local players here...
    std::uniform_int_distribution<int> dist(1, 12);
    int rand = dist(generator);
    std::vector<std::string> paths;
    paths.push_back("Audio/hurt/hurt" + std::to_string(rand) + ".wav");

    // Breathe
    int ammountOfbreaths = (static_cast<int>(e.Damage) / 10) + 2; // TEMP: Idk something stupid like this shit
    for (int i = 0; i < ammountOfbreaths; i++) {
        paths.push_back("Audio/exhausted/breath.wav");
    }
    Events::PlayQueueOnEntity ev;
    ev.Emitter = m_LocalPlayer;
    ev.FilePaths = paths;
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnPlayerDeath(const Events::PlayerDeath & e)
{
    if (e.PlayerID == m_LocalPlayer.ID) {
        Events::PlaySoundOnEntity ev;
        ev.EmitterID = createChildEmitter(m_LocalPlayer);
        ev.FilePath = "Audio/die/die2.wav"; // should random between a bunch
        m_EventBroker->Publish(ev);
    }
    return false;
}

bool SoundSystem::OnPlayerHealthPickup(const Events::PlayerHealthPickup & e)
{
    if (e.PlayerHealedID == m_LocalPlayer.ID) {
        Events::PlaySoundOnEntity ev;
        ev.EmitterID = createChildEmitter(m_LocalPlayer);
        ev.FilePath = "Audio/pickup/pickup2.wav";
        m_EventBroker->Publish(ev);
    }
    return false;
}

bool SoundSystem::OnTriggerTouch(const Events::TriggerTouch & e)
{
    if (m_World->HasComponent(e.Trigger.ID, "CapturePoint")) {
        Events::PlayBackgroundMusic ev;
        ev.FilePath = "Audio/bgm/drumstest.wav";
        m_EventBroker->Publish(ev);
    }
    return false;
}

bool SoundSystem::OnDoubleJump(const Events::DoubleJump & e)
{
    Events::PlaySoundOnEntity ev;
    ev.EmitterID = createChildEmitter(m_LocalPlayer);
    ev.FilePath = "Audio/jump/jump2.wav";
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnDashAbility(const Events::DashAbility &e)
{
    Events::PlaySoundOnEntity ev;
    ev.EmitterID = createChildEmitter(m_LocalPlayer);
    ev.FilePath = "Audio/jump/dash1.wav";
    m_EventBroker->Publish(ev);
    return false;
}

EntityID SoundSystem::createChildEmitter(EntityWrapper localPlayer)
{
    EntityID child = m_World->CreateEntity(localPlayer.ID);
    m_World->AttachComponent(child, "Transform");
    m_World->AttachComponent(child, "SoundEmitter");
    return child;
}