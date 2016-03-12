#include "Game/Systems/SoundSystem.h"

SoundSystem::SoundSystem(SystemParams params)
    : System(params)
    , PureSystem("SoundEmitter")
{
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    m_Announcer = ResourceManager::Load<ConfigFile>("Config.ini")->Get<std::string>("Sound.Announcer", "female");
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    m_RandomGenerator = std::default_random_engine(seed);
    m_RandIntDistribution = std::uniform_int_distribution<int>(1, 12);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &SoundSystem::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_InputCommand, &SoundSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EDoubleJump, &SoundSystem::OnDoubleJump);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &SoundSystem::OnPlayerDamage);
    EVENT_SUBSCRIBE_MEMBER(m_ECaptured, &SoundSystem::OnCaptured);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDeath, &SoundSystem::OnPlayerDeath);
}

void SoundSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt)
{ }

void SoundSystem::Update(double dt)
{ }

bool SoundSystem::OnPlayerSpawned(const Events::PlayerSpawned &e)
{
    if (e.PlayerID == -1) { // Local player
        m_World->AttachComponent(e.Player.ID, "Listener");
        Events::PlayAnonuncerVoice go;
        go.FilePath = "Audio/Announcer/" + m_Announcer + "/Go.wav";
        m_EventBroker->Publish(go);
        {
            Events::ChangeBGM ev;
            ev.FilePath = "Audio/BGM/Layer1.wav";
            m_EventBroker->Publish(ev);
        }
    }
    return true;
}

bool SoundSystem::OnInputCommand(const Events::InputCommand & e)
{
    if (e.Command == "Jump" && e.Value > 0) {
        if (e.PlayerID == -1) {
            playerJumps(LocalPlayer);
        } else {
            playerJumps(e.Player);
        }
        return true;
    }
    return false;
}

void SoundSystem::playerJumps(EntityWrapper player)
{
    if (!IsClient) { // Only play for clients
        return;
    }
    if(!player.HasComponent("Physics")) {
        return;
    }
    bool grounded = (bool)player["Physics"]["IsOnGround"];
    if (grounded) {
        Events::PlaySoundOnEntity e;
        e.Emitter = player;
        e.FilePath = "Audio/Jump/Jump1.wav";
        m_EventBroker->Publish(e);
    }
}

bool SoundSystem::OnCaptured(const Events::Captured & e)
{
    if (!IsClient) { // Only play for clients
        return false;
    }
    int homeTeam = (int)m_World->GetComponent(e.CapturePointTakenID, "Team")["Team"];
    int team = (int)m_World->GetComponent(LocalPlayer.ID, "Team")["Team"];
    Events::PlayAnonuncerVoice ev;
    if (team == homeTeam) {
        ev.FilePath = "Audio/Announcer/" + m_Announcer + "/ObjectiveAchieved.wav";
    } else {
        ev.FilePath = "Audio/Announcer/" + m_Announcer + "/ObjectiveFailed.wav";
    }
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnPlayerDamage(const Events::PlayerDamage & e)
{
    if (!IsClient) { // Only play for clients
        return false;
    }
    if (!e.Victim.Valid() || !e.Inflictor.Valid()) {
        return false;
    }
    auto victimTeam = m_World->GetComponent(e.Victim.ID, "Team");
    auto inflictorTeam = m_World->GetComponent(e.Inflictor.ID, "Team");
    if ((int)victimTeam["Team"] == (int)inflictorTeam["Team"]) {
        // Victim and inflictor are the same team, should not play "hurt" sound.
        return false;
    }
    std::vector<std::string> paths;
    paths.push_back("Audio/Hurt/Hurt" + std::to_string(m_RandIntDistribution(m_RandomGenerator)) + ".wav");

    Events::PlayQueueOnEntity ev;
    ev.Emitter = e.Victim;
    ev.FilePaths = paths;
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnPlayerDeath(const Events::PlayerDeath & e)
{
    if (!e.Player.Valid()) {
        return false;
    }
    if (!IsClient) {
        return false;
    }
    // The local player is dead. The local player might be invalid?
    // Play the sound from the listener.
    // TODO: We might want to hear other players die.
    Events::PlaySoundOnEntity ev;
    ev.Emitter = e.Player;
    ev.FilePath = "Audio/Die/Die2.wav";
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnPlayerHealthPickup(const Events::PlayerHealthPickup & e)
{
    Events::PlaySoundOnEntity ev;
    ev.Emitter = LocalPlayer;
    ev.FilePath = "Audio/Pickup/Pickup2.wav";
    m_EventBroker->Publish(ev);
    return false;
}

bool SoundSystem::OnDoubleJump(const Events::DoubleJump & e)
{
    if (!m_World->ValidEntity(e.entityID)) {
        return false;
    }
    if (!IsClient) {
        return false;
    }
    if (e.entityID == LocalPlayer.ID) {
        Events::PlaySoundOnEntity ev;
        ev.Emitter = EntityWrapper(m_World, e.entityID);
        ev.FilePath = "Audio/Jump/Jump2.wav";
        m_EventBroker->Publish(ev);
    }
    return false;
}