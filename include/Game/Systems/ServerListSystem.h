#ifndef ServerListSystem_h__
#define ServerListSystem_h__

#include "Core/System.h"
#include "Rendering/IRenderer.h"
#include "Core/ResourceManager.h"
#include "Core/Event.h"
#include "Systems/SpawnerSystem.h"
#include "Core/EventBroker.h"

#include "Network/ESearchForServers.h"
#include "Network/EDisplayServerlist.h"

class ServerListSystem : public PureSystem
{
public:
    ServerListSystem(SystemParams params, IRenderer* renderer);
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cServerList, double dt) override;

    void RefreshList();

private:
    IRenderer* m_Renderer;

    EventRelay<ServerListSystem, Events::DisplayServerlist> m_EServerListRecieved;
    bool OnServerListRecieved(const Events::DisplayServerlist& e);
};

#endif