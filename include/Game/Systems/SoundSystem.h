#ifndef Systems_SoundSystem_h__
#define Systems_SoundSystem_h__

#include "../Engine/Core/System.h"
#include "../Engine/Sound/SoundManager.h"
#include "../Engine/Core/EPlayerSpawned.h"
#include "../Engine/Input/EInputCommand.h"


class SoundSystem : public PureSystem, ImpureSystem, SoundManager
{
public:
    SoundSystem(World* world, EventBroker* eventbroker);
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt) override;
    virtual void Update(double dt) override;
private:
    void playerStep(double dt);
    EntityWrapper m_LocalPlayer = EntityWrapper();

    World* m_World = nullptr;
    EventBroker* m_EventBroker = nullptr;

    void playerJumps();


    // TODO: WIP Update this
    float m_DistanceMoved = 0.0f;
    const float m_PlayerStepLength = 2.0f;
    glm::vec3 m_LastPosition = glm::vec3();
    bool m_LeftFoot = false;

    EventRelay<SoundSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(const Events::PlayerSpawned &e);
    EventRelay<SoundSystem, Events::InputCommand> m_InputCommand;
    bool OnInputCommand(const Events::InputCommand &e);
};

#endif
