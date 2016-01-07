#include "Sound/SoundSystem.h"

SoundSystem::SoundSystem(World* world, EventBroker* eventBroker)
{
    m_EventBroker = eventBroker;
    m_World = world;
    // Initialize OpenAL
    m_ALCdevice = alcOpenDevice(nullptr);
    if (m_ALCdevice != nullptr) {
        m_ALCcontext = alcCreateContext(m_ALCdevice, nullptr);
        alcMakeContextCurrent(m_ALCcontext);
    } else {
        LOG_ERROR("OpenAL failed to initialize.");
    }

    alSpeedOfSound(340.29f); // Speed of sound
    alDistanceModel(AL_INVERSE_DISTANCE);

    EVENT_SUBSCRIBE_MEMBER(m_EPlaySound, &SoundSystem::OnPlaySound);
}

SoundSystem::~SoundSystem()
{
    alcDestroyContext(m_ALCcontext);
    alcCloseDevice(m_ALCdevice);
    delete m_ALCcontext;
    delete m_ALCdevice;
}

void SoundSystem::Update()
{
    // Should only be one listener.
    auto listenerComponents = m_World->GetComponents("Listener");
    for (auto it = listenerComponents->begin(); it != listenerComponents->end(); it++) {
        EntityID listener = (*it).EntityID;
        auto transform = m_World->GetComponent(listener, "Transform");
        setListenerPos(transform["Position"]);
    }
}

ALuint SoundSystem::createSource()
{
    ALuint source;
    alGenSources((ALuint)1, &source);
    alSourcei(source, AL_REFERENCE_DISTANCE, 1.0);
    alSourcei(source, AL_MAX_DISTANCE, FLT_MAX);
    return source;
}

void SoundSystem::playSound(Source source)
{
    alSourcei(source.ALsource, AL_BUFFER, source.SoundResource->Buffer());
    alSourcePlay(source.ALsource);
}

void SoundSystem::stopSound(Source source)
{ }

void SoundSystem::stopEmitter(EntityID emitter)
{ }

bool SoundSystem::OnPlaySound(const Events::PlaySound & e)
{
    Sound *sound = ResourceManager::Load<Sound>(e.FilePath);
    if (sound == nullptr) {
        return false;
    }
    ALuint source = createSource();
    Source sauce;
    sauce.ALsource = source;
    sauce.SoundResource = sound;
    playSound(sauce);
    return false;
}

