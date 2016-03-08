#include "../Game/Systems/ServerListSystem.h"

ServerListSystem::ServerListSystem(SystemParams params, IRenderer* renderer)
    : System(params)
    , PureSystem("ServerList")
    , m_Renderer(renderer)
{
    //Subscribe to OnServerListRecieved
}

void ServerListSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cServerList, double dt)
{
    
}

void ServerListSystem::RefreshList()
{
    //Send out request for a server list.
}

