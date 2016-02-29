#include "Game/Systems/SoundSystem.h"

SoundSystem::SoundSystem(SystemParams params)
    : System(params)
    , PureSystem("SoundEmitter")
{
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    m_Announcer = ResourceManager::Load<ConfigFile>("Config.ini")->Get<std::string>("Sound.Announcer", "female");
    if (IsClient) {
        EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &SoundSystem::OnPlayerSpawned);
        EVENT_SUBSCRIBE_MEMBER(m_InputCommand, &SoundSystem::OnInputCommand);
        EVENT_SUBSCRIBE_MEMBER(m_EDoubleJump, &SoundSystem::OnDoubleJump);
        EVENT_SUBSCRIBE_MEMBER(m_EDashAbility, &SoundSystem::OnDashAbility);
        EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &SoundSystem::OnPlayerDamage);
        EVENT_SUBSCRIBE_MEMBER(m_ECaptured, &SoundSystem::OnCaptured);
        EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &SoundSystem::OnTriggerTouch);
        EVENT_SUBSCRIBE_MEMBER(m_EPlayerDeath, &SoundSystem::OnPlayerDeath);
    }
}

void SoundSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt)
{ }

void SoundSystem::Update(double dt)
{
    if (!IsClient) {
        return;
    }
}

bool SoundSystem::OnPlayerSpawned(const Events::PlayerSpawned &e)
{
    if (e.PlayerID == -1) { // Local player
        m_World->AttachComponent(e.Player.ID, "Listener");
        Events::PlayAnonuncerVoice go;
        go.FilePath = "Audio/announcer/" + m_Announcer + "/go.wav";
        m_EventBroker->Publish(go);
        // TEMP: starts bgm
        {
            Events::PlayBackgroundMusic ev;
            ev.FilePath = "Audio/bgm/layer1.wav";
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

    return false;
}

void SoundSystem::playerJumps()
{
    if (!LocalPlayer.Valid()) {
        return;
    }

    bool grounded = (bool)m_World->GetComponent(LocalPlayer.ID, "Physics")["IsOnGround"];
    if (grounded) {
        Events::PlaySoundOnEntity e;
        e.EmitterID = LocalPlayer.ID;
        e.FilePath = "Audio/jump/jump1.wav";
        m_EventBroker->Publish(e);
    }
}

bool SoundSystem::OnCaptured(const Events::Captured & e)
{
    if (!LocalPlayer.Valid()) {
        return false;
    }
    int homeTeam = (int)m_World->GetComponent(e.CapturePointID, "Team")["Team"];
    int team = (int)m_World->GetComponent(LocalPlayer.ID, "Team")["Team"];
    Events::PlayAnonuncerVoice ev;
    if (team == homeTeam) {
        ev.FilePath = "Audio/announcer/" + m_Announcer + "/objective_achieved.wav";
    } else {
        ev.FilePath = "Audio/announcer/" + m_Announcer + "/objective_failed.wav"; // have not been tested
    }
    m_EventBroker->Publish(ev);
    return false;
}

// Testing purposes atm...
bool SoundSystem::OnPlayerDamage(const Events::PlayerDamage & e)
{
    if (!IsClient) { // Only play for clients
        return false;
    }
    if (LocalPlayer.ID = e.Victim.ID) { // You're local player was the one who took dmg
        return false;
    }
    std::uniform_int_distribution<int> dist(1, 12);
    int rand = dist(generator);
    std::vector<std::string> paths;
    paths.push_back("Audio/hurt/hurt" + std::to_string(rand) + ".wav");

    Events::PlayQueueOnEntity ev;
    ev.Emitter = LocalPlayer;
    ev.FilePaths = paths;
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnPlayerDeath(const Events::PlayerDeath & e)
{
    if (e.Player.ID != LocalPlayer.ID) {
        return false;
    }
    if (!IsClient) {
        return false;
    }
    // The local player is dead. The local player might be invalid?
    // Play the sound from the listener.
    // TODO: We might want to hear other players die.
    Events::PlayBackgroundMusic ev;
    ev.FilePath = "Audio/die/die2.wav";
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnPlayerHealthPickup(const Events::PlayerHealthPickup & e)
{
    Events::PlaySoundOnEntity ev;
    ev.EmitterID = LocalPlayer.ID;
    ev.FilePath = "Audio/pickup/pickup2.wav";
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnTriggerTouch(const Events::TriggerTouch & e)
{
    //// Temp for play test.
    //if (m_DrumsIsPlaying) {
    //    return false;
    //}
    //if (m_World->HasComponent(e.Trigger.ID, "CapturePoint")) {
    //    Events::PlaySoundOnEntity ev; // should be BGM
    //    ev.EmitterID = LocalPlayer.ID;
    //    ev.FilePath = "Audio/bgm/drumstest.wav";
    //    m_EventBroker->Publish(ev);
    //    // Temp for play test.
    //    m_DrumsIsPlaying = true;
    //}
    return false;
}

bool SoundSystem::OnDoubleJump(const Events::DoubleJump & e)
{
    Events::PlaySoundOnEntity ev;
    ev.EmitterID = LocalPlayer.ID;
    ev.FilePath = "Audio/jump/jump2.wav";
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnDashAbility(const Events::DashAbility &e)
{
    Events::PlaySoundOnEntity ev;
    ev.EmitterID = LocalPlayer.ID;
    ev.FilePath = "Audio/jump/dash1.wav";
    m_EventBroker->Publish(ev);
    return false;
}