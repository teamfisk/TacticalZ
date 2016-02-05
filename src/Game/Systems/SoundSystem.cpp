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
}

void SoundSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt)
{
    
}

void SoundSystem::Update(double dt)
{
    SoundManager::Update(dt);
    playerStep(dt);
}

void SoundSystem::playerStep(double dt)
{
    if (!m_LocalPlayer.Valid()) {
        return;
    }
    if (m_LocalPlayer.ID == EntityID_Invalid) {
        return;
    }

    m_TimeSinceLastFootstep += dt;
    glm::vec3 vel = (glm::vec3)m_World->GetComponent(m_LocalPlayer.ID, "Physics")["Velocity"];
    float playerSpeed = glm::length(vel);
    bool isAirborne = vel.y != 0;
    if (playerSpeed > 1 && !isAirborne) {
        // Player is walking
        if (m_TimeSinceLastFootstep * std::min<float>(playerSpeed, 2) > m_PlayerFootstepInterval) {
            // Create footstep sound
            Events::PlaySoundOnEntity e;
            e.EmitterID = createChildEmitter();
            if (m_LeftFoot) {
                e.FilePath = "Audio/footstep/footstep2.wav";
            } else {
                e.FilePath = "Audio/footstep/footstep3.wav";
            }
            m_LeftFoot = !m_LeftFoot;
            m_EventBroker->Publish(e);
            m_TimeSinceLastFootstep = 0;
        }
    }
}

bool SoundSystem::OnPlayerSpawned(const Events::PlayerSpawned &e)
{
    if (e.PlayerID == -1) { // Local player
        m_World->AttachComponent(e.Player.ID, "Listener");
        m_LocalPlayer = e.Player;
        Events::PlaySoundOnEntity event;
        event.EmitterID = createChildEmitter();
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
