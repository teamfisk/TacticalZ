#include "Sound/SoundSystem.h"

SoundSystem::SoundSystem(EventBroker* eventBroker)
{
    m_EventBroker = eventBroker;
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
    //m_EPlaySound = decltype(m_EPlaySound)(std::bind(&SoundSystem::OnPlaySound, this, std::placeholders::_1));
    //m_EventBroker->Subscribe(m_EPlaySound);
}

SoundSystem::~SoundSystem()
{ 
    alcDestroyContext(m_ALCcontext);
    alcCloseDevice(m_ALCdevice);
    delete m_ALCcontext;
    delete m_ALCdevice;
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
    LOG_INFO("You are playing an imaginary sound now! :D");
    return false;
}

