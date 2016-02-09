#ifndef System_h__
#define System_h__

#include "EventBroker.h"
#include "World.h"
#include "EntityWrapper.h"
#include "ComponentWrapper.h"
#include "EPlayerSpawned.h"

struct SystemParams
{
    SystemParams(::World* World, ::EventBroker* EventBroker, bool IsClient, bool IsServer)
        : World(World)
        , EventBroker(EventBroker)
        , IsClient(IsClient)
        , IsServer(IsServer)
    { }

    ::World* World;
    ::EventBroker* EventBroker;
    bool IsClient = false;
    bool IsServer = false;
};

class System
{
    friend class SystemPipeline;

protected:
    System(SystemParams params)
        : m_World(params.World)
        , m_EventBroker(params.EventBroker)
        , IsClient(params.IsClient)
        , IsServer(params.IsServer)
    {
        if (IsClient) {
            EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &System::OnPlayerSpawned);
        }
    }
    virtual ~System() = default;

    World* m_World;
    EventBroker* m_EventBroker;
    bool IsClient = false;
    bool IsServer = false;
    EntityWrapper LocalPlayer = EntityWrapper::Invalid;

private:
    EventRelay<System, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(Events::PlayerSpawned& e)
    {
        if (e.PlayerID == -1) {
            LocalPlayer = e.Player;
        }
        return true;
    }
};

class PureSystem : public virtual System
{
    friend class SystemPipeline;

protected:
    PureSystem(std::string componentType)
        : m_ComponentType(componentType)
    { }
    virtual ~PureSystem() = default;

    const std::string m_ComponentType;

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt) = 0;
};

class ImpureSystem : public virtual System
{
    friend class SystemPipeline;

protected:
    ImpureSystem() = default;
    virtual ~ImpureSystem() = default;

    virtual void Update(double dt) = 0;
};

#endif