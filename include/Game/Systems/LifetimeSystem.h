#ifndef LifetimeSystem_h__
#define LifetimeSystem_h__

#include "Core/System.h"

class LifetimeSystem : public ImpureSystem, PureSystem
{
public:
    LifetimeSystem(World* world, EventBroker* eventBroker)
        : System(world, eventBroker)
        , PureSystem("Lifetime")
    { }

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cLifetime, double dt) override;

private:
    std::vector<EntityWrapper> m_Deletions;
};

#endif