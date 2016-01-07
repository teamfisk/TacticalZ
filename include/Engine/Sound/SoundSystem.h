#ifndef SoundSystem_h__
#define SoundSystem_h__

#include <unordered_map>

#include "glm/common.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "OpenAL/al.h"
#include "OpenAL/alc.h"

#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Sound/Sound.h"
#include "Sound/EPlaySound.h"

struct Source
{
    Sound* SoundResource;
    ALuint ALsource;
};

class SoundSystem
{
public:
    SoundSystem() { }
    SoundSystem(World* world, EventBroker* eventBroker);
    ~SoundSystem();
    void Update(); // Update emitters
private:
    // Private setters and getters for working with glm
    void setListenerPos(glm::vec3 pos) { alListener3f(AL_POSITION, pos.x, pos.y, pos.z); };
    glm::vec3 listnerPos() { glm::vec3 pos; alGetListener3f(AL_POSITION, &pos.x, &pos.y, &pos.z); return pos; };
    void setListenerVel(glm::vec3 vel) { alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z); };
    glm::vec3 listenerVel() { glm::vec3 vel; alGetListener3f(AL_VELOCITY, &vel.x, &vel.y, &vel.z); return vel; };
    void setListenerOri(glm::vec3 ori) 
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
        ALfloat lori[6] = { forward.x, forward.y, forward.z , up.x, up.y, up.z };
        alListenerfv(AL_ORIENTATION, lori); 
    };
    glm::vec3 listenerOri() { glm::vec3 ori; alGetListener3f(AL_ORIENTATION, &ori.x, &ori.y, &ori.z); return ori; };
    void setSourcePos(ALuint source, glm::vec3 pos) { ALfloat spos[3] = { pos.x, pos.y, pos.z };  alSourcefv(source, AL_POSITION, spos); };
    void setSourceVel(ALuint source, glm::vec3 vel) { ALfloat svel[3] = { vel.x, vel.y, vel.z };  alSourcefv(source, AL_VELOCITY, svel); };
    void setSourceOri(ALuint source, glm::vec3 ori) { ALfloat sori[3] = { ori.x, ori.y, ori.z };  alSourcefv(source, AL_ORIENTATION, sori); };

    // Logic
    World* m_World;
    EventBroker* m_EventBroker;

    ALuint createSource();
    void playSound(Source source);
    void stopSound(Source source);
    void stopEmitter(EntityID emitter);

    // OpenAL system variables
    ALCdevice* m_ALCdevice = nullptr;
    ALCcontext* m_ALCcontext = nullptr;

    // Logic
    std::unordered_map<EntityID, Source> m_Sources;

    // Events
    EventRelay<SoundSystem, Events::PlaySound> m_EPlaySound;
    bool OnPlaySound(const Events::PlaySound &e);

};

#endif