#include "Sound/SoundSystem.h"

SoundSystem::SoundSystem(World* world, EventBroker* eventBroker)
{
    m_EventBroker = eventBroker;
    m_World = world;
    
    initOpenAL();

    alSpeedOfSound(340.29f); // Speed of sound
    alDistanceModel(AL_INVERSE_DISTANCE);

    EVENT_SUBSCRIBE_MEMBER(m_EPlaySound, &SoundSystem::OnPlaySound);
}

void SoundSystem::initOpenAL()
{
    // Initialize OpenAL
    m_ALCdevice = alcOpenDevice(nullptr);
    if (m_ALCdevice != nullptr) {
        m_ALCcontext = alcCreateContext(m_ALCdevice, nullptr);
        alcMakeContextCurrent(m_ALCcontext);
    } else {
        LOG_ERROR("OpenAL failed to initialize.");
    }
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
    //deleteInactiveEmitters(); // Not tested
    addNewEmitters();
    updateEmitters();
    updateListener();
}

void SoundSystem::deleteInactiveEmitters()
{
    auto emitterComponents = m_World->GetComponents("SoundEmitter");
    for (auto it = emitterComponents->begin(); it != emitterComponents->end(); it++) {
        EntityID emitter = (*it).EntityID;
        if (isPlaying(m_Sources[emitter].ALsource)) { // The sound is still playing, do not remove
            continue;
        }
        alDeleteBuffers(1, &m_Sources[emitter].ALsource);
        delete m_Sources[emitter].SoundResource;
        m_Sources.erase(emitter);
    }
}

void SoundSystem::addNewEmitters()
{ 
    auto emitterComponents = m_World->GetComponents("SoundEmitter");
    for (auto it = emitterComponents->begin(); it != emitterComponents->end(); it++) {
        EntityID emitter = (*it).EntityID;
        std::unordered_map<EntityID, Source>::iterator i;
        i = m_Sources.find(emitter);
        if (i == m_Sources.end()) { // Did not exist, add it
            Source source;
            source.ALsource = createSource();
            source.SoundResource = ResourceManager::Load<Sound>((std::string)(*it)["FilePath"]);
            m_Sources[emitter] = source;
        }
    }
}

void SoundSystem::updateEmitters()
{
    auto emitterComponents = m_World->GetComponents("SoundEmitter");
    for (auto it = emitterComponents->begin(); it != emitterComponents->end(); it++) {
        EntityID emitter = (*it).EntityID;
        std::unordered_map<EntityID, Source>::iterator i;
        i = m_Sources.find(emitter);
        if (i != m_Sources.end()) {
            setSourcePos(m_Sources[emitter].ALsource, RenderQueueFactory::AbsolutePosition(m_World, emitter));
        }
        // No orientation, emitts in all directions
        // Velocity for doppler effect
    }
}

void SoundSystem::updateListener()
{
    // Should only be one listener.
    auto listenerComponents = m_World->GetComponents("Listener");
    for (auto it = listenerComponents->begin(); it != listenerComponents->end(); it++) {
        EntityID listener = (*it).EntityID;
        setListenerPos(RenderQueueFactory::AbsolutePosition(m_World, listener));
        setListenerOri(glm::eulerAngles(RenderQueueFactory::AbsoluteOrientation(m_World, listener)));
        // Velocity for doppler effect
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
    //Sound *sound = ResourceManager::Load<Sound>(e.FilePath);
    //if (sound == nullptr) {
    //    return false;
    //}
    //ALuint source = createSource();
    //Source sauce;
    //sauce.ALsource = source;
    //sauce.SoundResource = sound;
    //playSound(sauce);
    return false;
}

void SoundSystem::setListenerOri(glm::vec3 ori)
{
    glm::vec3 forward = glm::vec3(0.0, 0.0, -1.0);
    forward = glm::rotateX(forward, ori.x);
    forward = glm::rotateY(forward, ori.y);
    forward = glm::rotateZ(forward, ori.z);
    glm::normalize(forward);
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    up = glm::rotateX(up, ori.x);
    up = glm::rotateY(up, ori.y);
    up = glm::rotateZ(up, ori.z);
    glm::normalize(up);
    ALfloat lOri[6] = { forward.x, forward.y, forward.z, up.x, up.y, up.z };
    alListenerfv(AL_ORIENTATION, lOri);
}

bool SoundSystem::isPlaying(ALuint source)
{
    ALenum state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING);
}

