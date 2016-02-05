#include "Game/Systems/SoundSystem.h"

SoundSystem::SoundSystem(World* world, EventBroker* eventbroker)
    : System(world, eventbroker)
    , PureSystem("SoundEmitter")
    , ImpureSystem()
    , SoundManager(world, eventbroker, true)
{
    m_World = world;
    m_EventBroker = eventbroker;
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &SoundSystem::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_InputCommand, &SoundSystem::OnInputCommand);
}

void SoundSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt)
{

}

void SoundSystem::Update(double dt)
{
    playerStep(dt);
    SoundManager::Update(dt);
}

void SoundSystem::playerStep(double dt)
{
    if (!m_LocalPlayer.Valid()) {
        return;
    }
    glm::vec3 pos = (glm::vec3)m_World->GetComponent(m_LocalPlayer.ID, "Transform")["Position"];
    glm::vec3 vel = (glm::vec3)m_World->GetComponent(m_LocalPlayer.ID, "Physics")["Velocity"];
    glm::vec3 difference = pos - m_LastPosition;
    m_LastPosition = pos;
    m_DistanceMoved += glm::length(difference);
    bool isAirborne = vel.y != 0;
    if (m_DistanceMoved > m_PlayerStepLength && !isAirborne) {
        // Player moved a step's distance
        // Create footstep sound
        Events::PlaySoundOnEntity e;
        e.EmitterID = createChildEmitter(m_LocalPlayer);
        e.FilePath = m_LeftFoot ? "Audio/footstep/footstep2.wav" : "Audio/footstep/footstep3.wav";
        m_LeftFoot = !m_LeftFoot;
        m_EventBroker->Publish(e);
        m_DistanceMoved = 0.f;
    }
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
//         {
//             Events::PlayBackgroundMusic ev;
//             ev.FilePath = "Audio/bgm/ambient.wav";
//             m_EventBroker->Publish(ev);
//         }
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
            // Reset the distance moved
            m_DistanceMoved = 0.f;
        }
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
