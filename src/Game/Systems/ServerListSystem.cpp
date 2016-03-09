#include "../Game/Systems/ServerListSystem.h"

ServerListSystem::ServerListSystem(SystemParams params, IRenderer* renderer)
    : System(params)
    , PureSystem("ServerList")
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EServerListRecieved, &ServerListSystem::OnServerListRecieved);
}

void ServerListSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cServerList, double dt)
{
    
}

void ServerListSystem::RefreshList()
{
    Events::SearchForServers event;
    m_EventBroker->Publish(event);
}


bool ServerListSystem::OnServerListRecieved(const Events::DisplayServerlist& e)
{
    if (e.Serverlist.size() == 0) {
        return 1;
    }
    auto serverLists = m_World->GetComponents("ServerList");
    if (serverLists == nullptr)
        return 1;
    for (auto& cServerList : *serverLists) {
        EntityWrapper serverListEntity = EntityWrapper(m_World, cServerList.EntityID);
        EntityWrapper identitySpawner = serverListEntity.FirstChildByName("ServerIdentitySpawner");
        identitySpawner.DeleteChildren();

        (int&)cServerList["TotalIdentities"] = (int)e.Serverlist.size();
        for (int i = 0; i < e.Serverlist.size(); i++) {
            //Create Identities for each server and place them on the right position.
            EntityWrapper newIdentity = SpawnerSystem::Spawn(identitySpawner, identitySpawner);
            EntityWrapper serverIdentityEntity = newIdentity.FirstChildByName("ServerIdentity");

            glm::vec3 offset = (glm::vec3)serverListEntity["ServerList"]["Offset"];
            (glm::vec3&)serverIdentityEntity["Transform"]["Position"] = offset * (float)i;

            auto& cIdentity = serverIdentityEntity["ServerIdentity"];
            (std::string&)cIdentity["IP"] = e.Serverlist[i].Address;
            (std::string&)cIdentity["ServerName"] = e.Serverlist[i].Name;
            (int&)cIdentity["Port"] = e.Serverlist[i].Port;
            (int&)cIdentity["PlayersConnected"] = e.Serverlist[i].PlayersConnected;
        }
    }

    return 1;
}
