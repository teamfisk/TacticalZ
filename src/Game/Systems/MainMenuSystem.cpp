#include "../Game/Systems/MainMenuSystem.h"

MainMenuSystem::MainMenuSystem(SystemParams params, IRenderer* renderer)
    : System(params)
    , ImpureSystem()
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPressed, &MainMenuSystem::OnButtonPress);
    EVENT_SUBSCRIBE_MEMBER(m_EReleased, &MainMenuSystem::OnButtonRelease);
    EVENT_SUBSCRIBE_MEMBER(m_EClicked, &MainMenuSystem::OnButtonClick);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &MainMenuSystem::OnInputCommand);
}

void MainMenuSystem::Update(double dt)
{

}


void MainMenuSystem::OnPlay(const Events::InputCommand& e)
{
    auto menus = m_World->GetComponents("Menu");
    if (menus == nullptr) {
        return;
    }

    if (m_OpenSubMenu == EntityWrapper::Invalid) {
        //No submenu is open, open one.
        for (auto& menu : *menus) {
            EntityWrapper menuEntity = EntityWrapper(m_World, menu.EntityID);
            auto serverListSpawner = menuEntity.FirstChildByName(e.Command + "Spawner");
            if (!serverListSpawner.HasComponent("Spawner")) {
                return;
            }
            m_OpenSubMenu = SpawnerSystem::Spawn(serverListSpawner, serverListSpawner);
            Events::SearchForServers event;
            m_EventBroker->Publish(event);
            break;
        }

    } else if (!m_OpenSubMenu.HasComponent("ServerList")) {
        //Menu is open, but not the right one, delete the old one and open a new one.
        m_World->DeleteEntity(m_OpenSubMenu.ID);
        m_OpenSubMenu = EntityWrapper::Invalid;

        for (auto& menu : *menus) {
            EntityWrapper menuEntity = EntityWrapper(m_World, menu.EntityID);
            auto serverListSpawner = menuEntity.FirstChildByName(e.Command + "Spawner");
            if (!serverListSpawner.HasComponent("Spawner")) {
                return;
            }
            m_OpenSubMenu = SpawnerSystem::Spawn(serverListSpawner, serverListSpawner);
            Events::SearchForServers event;
            m_EventBroker->Publish(event);
            break;
        }
    } else {
        //Serverlist submenu is open, close it.
        m_World->DeleteEntity(m_OpenSubMenu.ID);
        m_OpenSubMenu = EntityWrapper::Invalid;
    }
}


void MainMenuSystem::OpenSubMenu(const Events::InputCommand& e)
{
  
}

bool MainMenuSystem::OnButtonClick(const Events::ButtonClicked& e)
{
    if (e.EntityName == "ServerIdentityConnect") {
        EntityWrapper entity = e.Entity;
        EntityWrapper serverIdentityEntity = entity.FirstParentWithComponent("ServerIdentity");
        if(serverIdentityEntity.Valid()) {
            Events::ConnectRequest event;
            event.IP = (std::string)serverIdentityEntity["ServerIdentity"]["IP"];
            event.Port = (int)serverIdentityEntity["ServerIdentity"]["Port"];
            printf("\n ----Request Server Connect----\nIP: %s\nPort: %i\n ------------------------------", event.IP, event.Port);
            m_EventBroker->Publish(event);
        }
    }
    return true;
}

bool MainMenuSystem::OnButtonRelease(const Events::ButtonReleased& e)
{
    return true;
}

bool MainMenuSystem::OnButtonPress(const Events::ButtonPressed& e)
{
    return true;
}

bool MainMenuSystem::OnInputCommand(const Events::InputCommand& e)
{
    if(e.Command == "Play" && e.Value == 1) {
        OnPlay(e);
    } else if (e.Command == "RefreshServerList" && e.Value == 1){
        Events::SearchForServers event;
        m_EventBroker->Publish(event);
    } else if (e.Command == "Options" && e.Value == 1) {
        OnOptions(e);
    }
    return true;
}