#ifndef Systems_SoundSystem_h__
#define Systems_SoundSystem_h__

#include "../Engine/Core/System.h"
#include "../Engine/Sound/SoundManager.h"
#include "../Engine/Core/EPlayerSpawned.h"


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

    // TODO: WIP Update this
    double m_TimeSinceLastFootstep = 0.0;
    const double  m_PlayerFootstepInterval = 1.0;
    bool m_LeftFoot = false;

    EventRelay<SoundSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(const Events::PlayerSpawned &e);
};

#endif
