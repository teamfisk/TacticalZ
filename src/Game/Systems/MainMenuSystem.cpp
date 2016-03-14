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

void MainMenuSystem::OpenSubMenu(const Events::InputCommand& e)
{
    auto menus = m_World->GetComponents("Menu");
    if (menus == nullptr) {
        return;
    }

    if (m_OpenSubMenu == EntityWrapper::Invalid) {
        //No submenu is open, open one.
        for (auto& menu : *menus) {
            EntityWrapper menuEntity = EntityWrapper(m_World, menu.EntityID);
            auto spawner = menuEntity.FirstChildByName(e.Command + "Spawner");
            if (!spawner.HasComponent("Spawner")) {
                return;
            }
            m_OpenSubMenu = SpawnerSystem::Spawn(spawner, spawner);
            if (e.Command == "Play") {
                Events::SearchForServers event;
                m_EventBroker->Publish(event);
            }
            break;
        }

    } else if (m_OpenSubMenu.Name().compare(e.Command) != 0) {
        //Menu is open, but not the right one, delete the old one and open a new one.
        m_World->DeleteEntity(m_OpenSubMenu.ID);
        m_OpenSubMenu = EntityWrapper::Invalid;
        m_World->DeleteEntity(m_DropDown.ID);
        m_DropDown = EntityWrapper::Invalid;

        for (auto& menu : *menus) {
            EntityWrapper menuEntity = EntityWrapper(m_World, menu.EntityID);
            auto Spawner = menuEntity.FirstChildByName(e.Command + "Spawner");
            if (!Spawner.HasComponent("Spawner")) {
                return;
            }
            m_OpenSubMenu = SpawnerSystem::Spawn(Spawner, Spawner);
            if(e.Command == "Play") {
                Events::SearchForServers event;
                m_EventBroker->Publish(event);
            }
            break;
        }
    } else {
        //wanted submenu is open, close it.
        m_World->DeleteEntity(m_OpenSubMenu.ID);
        m_OpenSubMenu = EntityWrapper::Invalid;
        m_World->DeleteEntity(m_DropDown.ID);
        m_DropDown = EntityWrapper::Invalid;
    }
}


void MainMenuSystem::OpenDropDown(const Events::InputCommand& e)
{
    auto menus = m_World->GetComponents("Menu");
    if (menus == nullptr) {
        return;
    }

    if (m_DropDown == EntityWrapper::Invalid) {
        //No submenu is open, open one.
        for (auto& menu : *menus) {
            EntityWrapper menuEntity = EntityWrapper(m_World, menu.EntityID);
            auto spawner = menuEntity.FirstChildByName(e.Command + "Spawner");
            if (!spawner.HasComponent("Spawner")) {
                return;
            }
            m_DropDown = SpawnerSystem::Spawn(spawner, spawner);
            break;
        }
    } else {
        m_World->DeleteEntity(m_DropDown.ID);
        m_DropDown = EntityWrapper::Invalid;
    }
}

bool MainMenuSystem::OnButtonClick(const Events::ButtonClicked& e)
{
    EntityWrapper entity = e.Entity;
    if (entity.Name() == "ServerIdentityConnect") {
        EntityWrapper serverIdentityEntity = entity.FirstParentWithComponent("ServerIdentity");
        if(serverIdentityEntity.Valid()) {
            Events::ConnectRequest event;
            event.IP = (std::string)serverIdentityEntity["ServerIdentity"]["IP"];
            event.Port = (int)serverIdentityEntity["ServerIdentity"]["Port"];
            printf("\n ----Request Server Connect----\nIP: %s\nPort: %i\n ------------------------------", event.IP, event.Port);
            m_EventBroker->Publish(event);
        }
    } else if (entity.HasComponent("ConfigBtnResolution")) {

        m_Renderer->SetResolution(Rectangle((int)entity["ConfigBtnResolution"]["Width"], (int)entity["ConfigBtnResolution"]["Height"]));
        m_World->DeleteEntity(m_DropDown.ID);
        m_DropDown = EntityWrapper::Invalid;
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
        OpenSubMenu(e);
    } else if (e.Command == "RefreshServerList" && e.Value == 1){
        Events::SearchForServers event;
        m_EventBroker->Publish(event);
    } else if (e.Command == "Options" && e.Value == 1) {
        OpenSubMenu(e);
    } else if (e.Command == "Resolution" && e.Value == 1) {
        OpenDropDown(e);
    } else if (e.Command == "Quit" && e.Value == 1) {
        exit(EXIT_FAILURE);
    }
    return true;
}