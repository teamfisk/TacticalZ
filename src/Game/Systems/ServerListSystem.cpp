#include "../Game/Systems/ServerListSystem.h"

ServerListSystem::ServerListSystem(SystemParams params, IRenderer* renderer)
    : System(params)
    , PureSystem("ServerList")
    , m_Renderer(renderer)
{

}

void ServerListSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cServerList, double dt)
{
    
}

void ServerListSystem::RefreshList()
{

}
