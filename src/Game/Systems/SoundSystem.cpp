#include "Game/Systems/SoundSystem.h"

SoundSystem::SoundSystem(SystemParams params)
    : System(params)
    , PureSystem("SoundEmitter")
{
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    m_Announcer = ResourceManager::Load<ConfigFile>("Config.ini")->Get<std::string>("Sound.Announcer", "female");

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
        go.FilePath = "Audio/Announcer/" + m_Announcer + "/Go.wav";
        m_EventBroker->Publish(go);
        // TEMP: starts bgm
        {
            Events::PlayBackgroundMusic ev;
            ev.FilePath = "Audio/BGM/Layer1.wav";
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
    if (e.Command == "SoundTemp" && e.Value > 0) {
        EntityWrapper entity(m_World, m_World->CreateEntity());
        auto transform = m_World->AttachComponent(entity.ID, "Transform");
        (glm::vec3&)transform["Position"] = glm::vec3(0, 10, 0);
        (glm::vec3&)transform["Scale"] = glm::vec3(10, 10, 10);
        auto emitter = m_World->AttachComponent(entity.ID, "SoundEmitter");
        auto model = m_World->AttachComponent(entity.ID, "Model");
        (std::string&)model["Resource"] = "Models/Core/UnitCube.mesh";
        Events::PlaySoundOnEntity ev;
        ev.Emitter = entity;
        ev.FilePath = "Audio/crosscounter.wav";
        m_EventBroker->Publish(ev);
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
        e.Emitter = LocalPlayer;
        e.FilePath = "Audio/Jump/Jump1.wav";
        m_EventBroker->Publish(e);
    }
}

bool SoundSystem::OnCaptured(const Events::Captured & e)
{
    if (!LocalPlayer.Valid()) {
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

// Testing purposes atm...
bool SoundSystem::OnPlayerDamage(const Events::PlayerDamage & e)
{
    if (!IsClient) { // Only play for clients
        return false;
    }
    if(!e.Victim.Valid()) {
        return false;
    }
    std::uniform_int_distribution<int> dist(1, 12);
    int rand = dist(generator);
    std::vector<std::string> paths;
    paths.push_back("Audio/Hurt/Hurt" + std::to_string(rand) + ".wav");

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
    Events::PlaySoundOnEntity ev;
    ev.Emitter = EntityWrapper(m_World, e.entityID);
    ev.FilePath = "Audio/Jump/Jump2.wav";
    m_EventBroker->Publish(ev);
    return false;
}