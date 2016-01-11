#include "Sound/SoundSystem.h"

SoundSystem::SoundSystem(World* world, EventBroker* eventBroker)
{
    m_EventBroker = eventBroker;
    m_World = world;
    
    initOpenAL();

    alSpeedOfSound(340.29f); // Speed of sound
    alDistanceModel(AL_LINEAR_DISTANCE);
    alDopplerFactor(1);

    EVENT_SUBSCRIBE_MEMBER(m_EPlaySound, &SoundSystem::OnPlaySound);
    EVENT_SUBSCRIBE_MEMBER(m_EPlaySoundOnEntity, &SoundSystem::OnPlaySoundOnEntity);
    EVENT_SUBSCRIBE_MEMBER(m_EPlaySoundOnPosition, &SoundSystem::OnPlaySoundOnPosition);
    EVENT_SUBSCRIBE_MEMBER(m_EStopSound, &SoundSystem::OnStopSound);
    EVENT_SUBSCRIBE_MEMBER(m_EPauseSound, &SoundSystem::OnPauseSound);
    EVENT_SUBSCRIBE_MEMBER(m_EContinueSound, &SoundSystem::OnContinueSound);
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
    stopEmitters(); // Stopps emitters
    deleteInactiveEmitters(); // Deletes stopped emitters

    std::unordered_map<EntityID, Source*>::iterator it;
    for (it = m_Sources.begin(); it != m_Sources.end(); it++) {
        m_World->DeleteEntity((*it).first);
    }
    m_Sources.clear();

    alcDestroyContext(m_ALCcontext);
    alcCloseDevice(m_ALCdevice);
    delete m_ALCcontext;
    delete m_ALCdevice;
}

void SoundSystem::stopEmitters()
{
    std::unordered_map<EntityID, Source*>::iterator it;
    for (it = m_Sources.begin(); it != m_Sources.end(); it++) {
        if (isPlaying((*it).second->ALsource)) {
            stopSound((*it).second);
        }
    }
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
    for (auto it = emitterComponents->begin(); it != emitterComponents->end();) {
        EntityID emitter = (*it).EntityID;
        if (isPlaying(m_Sources[emitter]->ALsource)) { // The sound is still playing, do not remove
            it++;
            continue;
        }
        else {
            alDeleteBuffers(1, &m_Sources[emitter]->ALsource);
            alDeleteSources(1, &m_Sources[emitter]->ALsource);
            //delete m_Sources[emitter]->SoundResource;
            m_Sources.erase(emitter);
            m_World->DeleteEntity(emitter);
        }
    }
}

void SoundSystem::addNewEmitters()
{ 
    auto emitterComponents = m_World->GetComponents("SoundEmitter");
    for (auto it = emitterComponents->begin(); it != emitterComponents->end(); it++) {
        EntityID emitter = (*it).EntityID;
        std::unordered_map<EntityID, Source*>::iterator i;
        i = m_Sources.find(emitter);
        if (i == m_Sources.end()) { // Did not exist, add it
            Source* source = createSource((std::string)(*it)["FilePath"]);
            setSoundProperties (
                source->ALsource,
                (float)(double)(*it)["Gain"],
                (float)(double)(*it)["Pitch"],
                (bool)(*it)["Loop"],
                (float)(double)(*it)["MaxDistance"],
                (float)(double)(*it)["RollOffFactor"],
                (float)(double)(*it)["ReferenceDistance"]
            );
            m_Sources[emitter] = source;
        }
    }
}

void SoundSystem::updateEmitters()
{
    auto emitterComponents = m_World->GetComponents("SoundEmitter");
    for (auto it = emitterComponents->begin(); it != emitterComponents->end(); it++) {
        EntityID emitter = (*it).EntityID;
        std::unordered_map<EntityID, Source*>::iterator i;
        i = m_Sources.find(emitter);
        if (i != m_Sources.end()) {
            glm::vec3 previousPos;
            alGetSource3f(m_Sources[emitter]->ALsource, AL_POSITION, &previousPos.x, &previousPos.y, &previousPos.z); // Get previous pos
            glm::vec3 nextPos = RenderQueueFactory::AbsolutePosition(m_World, emitter); // Get next pos
            glm::vec3 velocity = nextPos - previousPos; // Calculate velocity
            setSourcePos(m_Sources[emitter]->ALsource, nextPos); // Set next pos
            setSourceVel(m_Sources[emitter]->ALsource, velocity); // Set velocity
            setSoundProperties(
                m_Sources[emitter]->ALsource,
                (float)(double)(*it)["Gain"],
                (float)(double)(*it)["Pitch"],
                (bool)(*it)["Loop"],
                (float)(double)(*it)["MaxDistance"],
                (float)(double)(*it)["RollOffFactor"],
                (float)(double)(*it)["ReferenceDistance"]
            );
        }
        // No orientation, emitts in all directions
    }
}

void SoundSystem::updateListener()
{
    // Should only be one listener.
    auto listenerComponents = m_World->GetComponents("Listener");
    for (auto it = listenerComponents->begin(); it != listenerComponents->end(); it++) {
        EntityID listener = (*it).EntityID;
        glm::vec3 previousPos;
        alGetListener3f(AL_POSITION, &previousPos.x, &previousPos.y, &previousPos.z); // Get previous pos
        glm::vec3 nextPos = RenderQueueFactory::AbsolutePosition(m_World, listener); // Get next (current) pos
        glm::vec3 velocity = nextPos - previousPos; // Calculate velocity
        setListenerPos(nextPos);
        setListenerVel(velocity);
        setListenerOri(glm::eulerAngles(RenderQueueFactory::AbsoluteOrientation(m_World, listener)));
    }
}

Source* SoundSystem::createSource(std::string filePath)
{
    ALuint alSource;
    alGenSources((ALuint)1, &alSource);
    alSourcei(alSource, AL_REFERENCE_DISTANCE, 1.0);
    alSourcei(alSource, AL_MAX_DISTANCE, FLT_MAX);
    Source* source = new Source();
    source->ALsource = alSource;
    source->SoundResource = ResourceManager::Load<Sound>(filePath);
    return source;
}

void SoundSystem::playSound(Source* source)
{
    alSourcei(source->ALsource, AL_BUFFER, source->SoundResource->Buffer());
    alSourcePlay(source->ALsource);
}

void SoundSystem::stopSound(Source* source)
{ 
    alSourceStop(source->ALsource);
}

void SoundSystem::stopEmitter(EntityID emitter)
{ 
    
}

bool SoundSystem::OnPlaySound(const Events::PlaySound & e)
{
    Source* sauce = createSource(e.FilePath);
    playSound(sauce);
    return false;
}

bool SoundSystem::OnPlaySoundOnEntity(const Events::PlaySoundOnEntity & e)
{
    Source* source = createSource(e.FilePath);
    m_Sources[e.emitterID] = source;
    playSound(source);
    return false;
}

bool SoundSystem::OnPlaySoundOnPosition(const Events::PlaySoundOnPosition & e)
{
    Source* source = createSource(e.FilePath);
    auto emitterID = m_World->CreateEntity();
    auto transform = m_World->AttachComponent(emitterID, "Transform");
    (glm::vec3&)transform["Position"] = e.Position;
    auto emitter = m_World->AttachComponent(emitterID, "SoundEmitter");
    (float&)(double)emitter["Gain"] = e.Gain;
    (float&)(double)emitter["Pitch"] = e.Pitch;
    (bool&)emitter["Loop"] = e.Loop;
    (float&)(double)emitter["MaxDistance"] = e.MaxDistance;
    (float&)(double)emitter["RollOffFactor"] = e.RollOffFactor;
    (float&)(double)emitter["ReferenceDistance"] = e.ReferenceDistance;
    auto model = m_World->AttachComponent(emitterID, "Model");
    (std::string&)model["Resource"] = "Models/Core/UnitCube.obj";
    m_Sources[emitterID] = source;
    playSound(source);
    return false;
}

bool SoundSystem::OnPauseSound(const Events::PauseSound & e)
{
    alSourcePause(m_Sources[e.EmitterID]->ALsource);
    return false;
}

bool SoundSystem::OnStopSound(const Events::StopSound & e)
{
    alSourceStop(m_Sources[e.EmitterID]->ALsource);
    return false;
}

bool SoundSystem::OnContinueSound(const Events::ContinueSound & e)
{
    alSourcePlay(m_Sources[e.EmitterID]->ALsource);
    return true;
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

void SoundSystem::setSoundProperties(ALuint source, float gain, float pitch, bool loop, float maxDistance, float rollOffFactor, float referenceDistance)
{
    alSourcef(source, AL_GAIN, gain);
    alSourcef(source, AL_PITCH, pitch);
    alSourcei(source, AL_LOOPING, (int)loop); // YOLO
    alSourcef(source, AL_MAX_DISTANCE, maxDistance);
    alSourcef(source, AL_ROLLOFF_FACTOR, rollOffFactor);
    alSourcef(source, AL_REFERENCE_DISTANCE, referenceDistance);
}
