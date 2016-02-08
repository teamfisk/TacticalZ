#include "Sound/SoundManager.h"

SoundManager::SoundManager(World* world, EventBroker* eventBroker, bool editorMode)
{
    m_EventBroker = eventBroker;
    m_World = world;
    m_EditorEnabled = editorMode;

    initOpenAL();

    alSpeedOfSound(340.29f);
    alDistanceModel(AL_LINEAR_DISTANCE);
    alDopplerFactor(1);

    EVENT_SUBSCRIBE_MEMBER(m_EPlaySoundOnEntity, &SoundManager::OnPlaySoundOnEntity);
    EVENT_SUBSCRIBE_MEMBER(m_EPlaySoundOnPosition, &SoundManager::OnPlaySoundOnPosition);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayBackgroundMusic, &SoundManager::OnPlayBackgroundMusic);
    EVENT_SUBSCRIBE_MEMBER(m_EStopSound, &SoundManager::OnStopSound);
    EVENT_SUBSCRIBE_MEMBER(m_EPauseSound, &SoundManager::OnPauseSound);
    EVENT_SUBSCRIBE_MEMBER(m_EContinueSound, &SoundManager::OnContinueSound);
    EVENT_SUBSCRIBE_MEMBER(m_ESetBGMGain, &SoundManager::OnSetBGMGain);
    EVENT_SUBSCRIBE_MEMBER(m_ESetSFXGain, &SoundManager::OnSetSFXGain);
    EVENT_SUBSCRIBE_MEMBER(m_EPause, &SoundManager::OnPause);
    EVENT_SUBSCRIBE_MEMBER(m_EResume, &SoundManager::OnResume);
    EVENT_SUBSCRIBE_MEMBER(m_EComponentAttached, &SoundManager::OnComponentAttached);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &SoundManager::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayQueueOnEntity, &SoundManager::OnPlayQueueOnEntity);
}

SoundManager::~SoundManager()
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

void SoundManager::stopEmitters()
{
    std::unordered_map<EntityID, Source*>::iterator it;
    for (it = m_Sources.begin(); it != m_Sources.end(); it++) {
        if (getSourceState(it->second->ALsource) == AL_PLAYING) {
            stopSound(it->second);
        }
    }
}

void SoundManager::Update(double dt)
{
    m_EventBroker->Process<SoundManager>();
    deleteInactiveEmitters(); // can be optimized with "EEntityDeleted"
    updateEmitters(dt);
    updateListener(dt);

    // Editor debug info
    ImGui::SliderFloat("BGM", &m_BGMVolumeChannel, 0.0f, 1.0f, "%.3f", 1.0f);
    ImGui::SliderFloat("SFX", &m_SFXVolumeChannel, 0.0f, 1.0f, "%.3f", 1.0f);
}

void SoundManager::deleteInactiveEmitters()
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
                delete it->second;
                it = m_Sources.erase(it);
            }
        } else {
            // Entity / Component has been removed
            stopSound(it->second);
            alDeleteBuffers(1, &it->second->ALsource);
            alDeleteSources(1, &it->second->ALsource);
            delete it->second;
            it = m_Sources.erase(it);
        }
    }
}

void SoundManager::updateEmitters(double dt)
{
    std::unordered_map<EntityID, Source*>::iterator it;
    for (it = m_Sources.begin(); it != m_Sources.end(); it++) {
        // Get previous pos
        if (!m_World->ValidEntity(it->first)) {
            return;
        }
        glm::vec3 previousPos;
        alGetSource3f(it->second->ALsource, AL_POSITION, &previousPos.x, &previousPos.y, &previousPos.z);
        // Get next pos
        glm::vec3 nextPos = Transform::AbsolutePosition(m_World, it->first);
        // Calculate velocity
        glm::vec3 velocity = glm::vec3(nextPos - previousPos) / (float)dt;
        setSourcePos(it->second->ALsource, nextPos);
        setSourceVel(it->second->ALsource, velocity);

        auto emitter = m_World->GetComponent(it->first, "SoundEmitter");
        setSoundProperties(it->second, &emitter);

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

void SoundManager::updateListener(double dt)
{
    // Should only be one listener.
    auto listenerComponents = m_World->GetComponents("Listener");
    if (listenerComponents == nullptr || !m_LocalPlayer.Valid()) {
        return;
    }
    for (auto it = listenerComponents->begin(); it != listenerComponents->end(); it++) {
        EntityWrapper listener(m_World, (*it).EntityID);
        if (listener.IsChildOf(m_LocalPlayer) || listener == m_LocalPlayer) {
            glm::vec3 previousPos;
            alGetListener3f(AL_POSITION, &previousPos.x, &previousPos.y, &previousPos.z); // Get previous pos
            glm::vec3 nextPos = Transform::AbsolutePosition(listener); // Get next (current) pos
            glm::vec3 velocity = glm::vec3(nextPos - previousPos) / (float)dt; // Calculate velocity
            setListenerPos(nextPos);
            setListenerVel(velocity);
            setListenerOri(glm::eulerAngles(Transform::AbsoluteOrientation(listener)));
            break;
        }
    }
}

Source* SoundManager::createSource(std::string filePath)
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

void SoundManager::playSound(Source* source)
{
    alSourcei(source->ALsource, AL_BUFFER, source->SoundResource->Buffer());
    alSourcePlay(source->ALsource);
}

void SoundManager::playQueue(QueuedBuffers qb)
{
    for (int i = 0; i < qb.second.size(); i++) {
        alSourceQueueBuffers(qb.first, 1, &qb.second[i]);
    }
    alSourcePlay(qb.first);
}

void SoundManager::stopSound(Source* source)
{
    alSourceStop(source->ALsource);
}

bool SoundManager::OnPlaySoundOnEntity(const Events::PlaySoundOnEntity & e)
{
    Source* source = createSource(e.FilePath);
    source->Type = SoundType::SFX;
    m_Sources[e.EmitterID] = source;
    playSound(source);
    return false;
}

bool SoundManager::OnPlaySoundOnPosition(const Events::PlaySoundOnPosition & e)
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
    source->Type = SoundType::SFX;
    m_Sources[emitterID] = source;
    playSound(source);
    return true;
}

bool SoundManager::OnPauseSound(const Events::PauseSound & e)
{
    alSourcePause(m_Sources[e.EmitterID]->ALsource);
    return true;
}

bool SoundManager::OnStopSound(const Events::StopSound & e)
{
    alSourceStop(m_Sources[e.EmitterID]->ALsource);
    return true;
}

bool SoundManager::OnContinueSound(const Events::ContinueSound & e)
{
    alSourcePlay(m_Sources[e.EmitterID]->ALsource);
    return true;
}

bool SoundManager::OnPlayBackgroundMusic(const Events::PlayBackgroundMusic & e)
{
    auto listenerComponents = m_World->GetComponents("Listener");
    for (auto it = listenerComponents->begin(); it != listenerComponents->end(); it++) {
        auto emitterChild = m_World->CreateEntity((*it).EntityID);
        auto emitter = m_World->AttachComponent(emitterChild, "SoundEmitter");
        (bool&)emitter["Loop"] = false;
        (std::string&)emitter["FilePath"] = e.FilePath;
        m_World->AttachComponent(emitterChild, "Transform");
        Source* source = createSource(e.FilePath);
        source->Type = SoundType::BGM;
        setSoundProperties(source, &emitter);
        m_Sources[emitterChild] = source;
        playSound(source);
    }
    return true;
}

bool SoundManager::OnSetBGMGain(const Events::SetBGMGain & e)
{
    m_BGMVolumeChannel = e.Gain;
    return true;
}

bool SoundManager::OnSetSFXGain(const Events::SetSFXGain & e)
{
    m_SFXVolumeChannel = e.Gain;
    return true;
}

bool SoundManager::OnComponentAttached(const Events::ComponentAttached & e)
{
    if (e.Component.Info.Name == "SoundEmitter") {
        auto component = m_World->GetComponent(e.Entity.ID, "SoundEmitter");
        Source* source = createSource(component["FilePath"]);
        m_Sources[e.Entity.ID] = source;
    }
    return false;
}

bool SoundManager::OnPause(const Events::Pause & e)
{
    for (auto it = m_Sources.begin(); it != m_Sources.end(); it++) {
        alSourcePause(it->second->ALsource);
    }
    return false;
}

bool SoundManager::OnResume(const Events::Resume &e)
{
    for (auto it = m_Sources.begin(); it != m_Sources.end(); it++) {
        alSourcePlay(it->second->ALsource);
    }
    return false;
}


bool SoundManager::OnPlayerSpawned(const Events::PlayerSpawned &e)
{
    if (e.PlayerID == -1) { // Local player
        m_LocalPlayer = e.Player;
        return true;
    }
    return false;
}


bool SoundManager::OnPlayQueueOnEntity(const Events::PlayQueueOnEntity &e)
{
    Source* source = createSource(*e.FilePaths.begin());
    std::vector<ALuint> buffers;
    buffers.push_back(source->SoundResource->Buffer());
    source->Type = SoundType::BGM;
    std::vector<std::string>::const_iterator it;
    for (it = e.FilePaths.begin() + 1; it != e.FilePaths.end(); it++) {
        buffers.push_back(ResourceManager::Load<Sound>(*it)->Buffer());
    }
    playQueue(QueuedBuffers(source->ALsource, buffers));
    return true;
}

ALenum SoundManager::getSourceState(ALuint source)
{
    ALenum state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state;
}

void SoundManager::setGain(Source * source, float gain)
{
    alSourcef(source->ALsource, AL_GAIN, gain);
}

void SoundManager::setSoundProperties(Source* source, ComponentWrapper* soundComponent)
{
    float gain = (source->Type == SoundType::SFX) ? m_SFXVolumeChannel : m_BGMVolumeChannel;
    alSourcef(source->ALsource, AL_GAIN, (float)(double)(*soundComponent)["Gain"] * gain);
    alSourcef(source->ALsource, AL_PITCH, (float)(double)(*soundComponent)["Pitch"]);
    alSourcei(source->ALsource, AL_LOOPING, (int)(bool)(*soundComponent)["Loop"]); // YOLO
    alSourcef(source->ALsource, AL_MAX_DISTANCE, (float)(double)(*soundComponent)["MaxDistance"]);
    alSourcef(source->ALsource, AL_ROLLOFF_FACTOR, (float)(double)(*soundComponent)["RollOffFactor"]);
    alSourcef(source->ALsource, AL_REFERENCE_DISTANCE, (float)(double)(*soundComponent)["ReferenceDistance"]);
}

void SoundManager::initOpenAL()
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

void SoundManager::setListenerOri(glm::vec3 ori)
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