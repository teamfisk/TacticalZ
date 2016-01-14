#include "Sound/SoundSystem.h"

SoundSystem::SoundSystem(World* world, EventBroker* eventBroker, bool editorMode)
{
    m_EventBroker = eventBroker;
    m_World = world;
    m_EditorEnabled = editorMode;

    initOpenAL();

    alSpeedOfSound(340.29f);
    alDistanceModel(AL_LINEAR_DISTANCE);
    alDopplerFactor(1);

    EVENT_SUBSCRIBE_MEMBER(m_EPlaySoundOnEntity, &SoundSystem::OnPlaySoundOnEntity);
    EVENT_SUBSCRIBE_MEMBER(m_EPlaySoundOnPosition, &SoundSystem::OnPlaySoundOnPosition);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayBackgroundMusic, &SoundSystem::OnPlayBackgroundMusic);
    EVENT_SUBSCRIBE_MEMBER(m_EStopSound, &SoundSystem::OnStopSound);
    EVENT_SUBSCRIBE_MEMBER(m_EPauseSound, &SoundSystem::OnPauseSound);
    EVENT_SUBSCRIBE_MEMBER(m_EContinueSound, &SoundSystem::OnContinueSound);
    EVENT_SUBSCRIBE_MEMBER(m_ESetBGMGain, &SoundSystem::OnSetBGMGain);
    EVENT_SUBSCRIBE_MEMBER(m_ESetSFXGain, &SoundSystem::OnSetSFXGain);
}

SoundSystem::~SoundSystem()
{
    stopEmitters(); // Stopps emitters
    deleteInactiveEmitters(); // Deletes stopped emitters
    // Delete entities
    std::unordered_map<EntityID, Source*>::iterator it;
    for (it = m_Sources.begin(); it != m_Sources.end(); it++) {
        m_World->DeleteEntity((*it).first);
    }
    m_Sources.clear();

    alcDestroyContext(m_ALCcontext);
    alcCloseDevice(m_ALCdevice);
}

void SoundSystem::stopEmitters()
{
    std::unordered_map<EntityID, Source*>::iterator it;
    for (it = m_Sources.begin(); it != m_Sources.end(); it++) {
        if (getSourceState((*it).second->ALsource) == AL_PLAYING) {
            stopSound((*it).second);
        }
    }
}

void SoundSystem::Update()
{ 
    m_EventBroker->Process<SoundSystem>();
    addNewEmitters(); // can be optimized with "EEntityCreated"
    deleteInactiveEmitters(); // can be optimized with "EEntityDeleted"
    updateEmitters();
    updateListener();
}

void SoundSystem::deleteInactiveEmitters()
{
    std::unordered_map<EntityID, Source*>::iterator it;
    for (it = m_Sources.begin(); it != m_Sources.end();) {
        if (m_World->ValidEntity(it->first) 
            && m_World->HasComponent(it->first, "SoundEmitter")) {
            if (getSourceState(it->second->ALsource) != AL_STOPPED) {
                // Nothing to see here, move along
                it++;
                continue;
            } else {
                // Sound has been stopped / finished playing. 
                alDeleteBuffers(1, &it->second->ALsource);
                alDeleteSources(1, &it->second->ALsource);
                m_World->DeleteEntity(it->first);
                it = m_Sources.erase(it);
            }
        } else {
            // Entity / Component has been removed
            stopSound((*it).second);
            alDeleteBuffers(1, &it->second->ALsource);
            alDeleteSources(1, &it->second->ALsource);
            it = m_Sources.erase(it);
        }
    }
}

void SoundSystem::addNewEmitters()
{
    auto emitterComponents = m_World->GetComponents("SoundEmitter");
    if (emitterComponents == nullptr) {
        return;
    }
    for (auto it = emitterComponents->begin(); it != emitterComponents->end(); it++) {
        EntityID emitter = (*it).EntityID;
        std::unordered_map<EntityID, Source*>::iterator source;
        source = m_Sources.find(emitter);
        if (source == m_Sources.end()) { // Did not exist, add it
            Source* source = createSource((std::string)(*it)["FilePath"]);
            m_Sources[emitter] = source;
        }
    }
}

void SoundSystem::updateEmitters()
{
    std::unordered_map<EntityID, Source*>::iterator it;
    for (it = m_Sources.begin(); it != m_Sources.end(); it++) {
        // Get previous pos
        glm::vec3 previousPos;
        alGetSource3f(it->second->ALsource, AL_POSITION, &previousPos.x, &previousPos.y, &previousPos.z);
        // Get next pos
        glm::vec3 nextPos = RenderQueueFactory::AbsolutePosition(m_World, it->first);
        // Calculate velocity
        glm::vec3 velocity = nextPos - previousPos;
        setSourcePos(it->second->ALsource, nextPos);
        setSourceVel(it->second->ALsource, velocity);
        float gain;
        (bool)(it->second->Type) ? gain = m_SFXVolumeChannel : gain = m_BGMVolumeChannel;
        auto emitter = m_World->GetComponent(it->first, "SoundEmitter");
        setSoundProperties(it->second->ALsource, &emitter);

        // To make an emitter play when spawned in editor mode
        if (m_EditorEnabled) {
            // Path changed
            if (it->second->SoundResource->Path() != (std::string)emitter["FilePath"]) {
                it->second->SoundResource = ResourceManager::Load<Sound>((std::string)emitter["FilePath"]);
                if (it->second->SoundResource->Buffer() != 0) {
                    playSound(it->second);
                }
            }
        }
    }
}

void SoundSystem::updateListener()
{
    // Should only be one listener.
    auto listenerComponents = m_World->GetComponents("Listener");
    if (listenerComponents == nullptr) {
        return;
    }
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
    alSourcef(alSource, AL_REFERENCE_DISTANCE, 1.0);
    alSourcef(alSource, AL_MAX_DISTANCE, FLT_MAX);
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

bool SoundSystem::OnPlaySoundOnEntity(const Events::PlaySoundOnEntity & e)
{
    Source* source = createSource(e.FilePath);
    source->Type = SoundType::SFX;
    m_Sources[e.EmitterID] = source;
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
    source->Type = SoundType::SFX;
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

bool SoundSystem::OnPlayBackgroundMusic(const Events::PlayBackgroundMusic & e)
{
    auto listenerComponents = m_World->GetComponents("Listener");
    for (auto it = listenerComponents->begin(); it != listenerComponents->end(); it++) {
        auto emitterChild = m_World->CreateEntity((*it).EntityID);
        auto emitter = m_World->AttachComponent(emitterChild, "SoundEmitter");
        (bool&)emitter["Loop"] = true;
        (std::string&)emitter["FilePath"] = e.FilePath;
        m_World->AttachComponent(emitterChild, "Transform");
        Source* source = createSource(e.FilePath);
        source->Type = SoundType::BGM;
        m_Sources[emitterChild] = source;
        playSound(source);
    }
    return false;
}

bool SoundSystem::OnSetBGMGain(const Events::SetBGMGain & e)
{
    m_BGMVolumeChannel = e.Gain;
    return true;
}

bool SoundSystem::OnSetSFXGain(const Events::SetSFXGain & e)
{
    m_SFXVolumeChannel = e.Gain;
    return true;
}

void SoundSystem::setListenerOri(glm::vec3 ori)
{
    // Calculate forward and up vector.
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

ALenum SoundSystem::getSourceState(ALuint source)
{
    ALenum state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state;
}

void SoundSystem::setGain(Source * source, float gain)
{
    alSourcef(source->ALsource, AL_GAIN, gain);
}

void SoundSystem::setSoundProperties(ALuint source, ComponentWrapper* soundComponent)
{
    alSourcef(source, AL_GAIN, (float)(double)(*soundComponent)["Gain"]);
    alSourcef(source, AL_PITCH, (float)(double)(*soundComponent)["Pitch"]);
    alSourcei(source, AL_LOOPING, (int)(bool)(*soundComponent)["Loop"]); // YOLO
    alSourcef(source, AL_MAX_DISTANCE, (float)(double)(*soundComponent)["MaxDistance"]);
    alSourcef(source, AL_ROLLOFF_FACTOR, (float)(double)(*soundComponent)["RollOffFactor"]);
    alSourcef(source, AL_REFERENCE_DISTANCE, (float)(double)(*soundComponent)["ReferenceDistance"]);
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