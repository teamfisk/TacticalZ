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

bool MainMenuSystem::OnButtonClick(const Events::ButtonClicked& e)
{
    if(e.EntityName == "Play") {
        //OpenServerList

    } else if(e.EntityName == "Connect") {
        //Run connect code
    } else if(e.EntityName == "Host") {
        //Run host code
    } else if(e.EntityName == "Quit") {
        printf("No, you stay");
    } else if (e.EntityName == "Res1080") {
        glfwSetWindowSize(m_Renderer->Window(), 1920, 1080);
    } else if (e.EntityName == "Res720") {
        glfwSetWindowSize(m_Renderer->Window(), 1280, 720);
        glViewport(0, 0, 1280, 720);
    } else if (e.EntityName == "Res480") {
        glfwSetWindowSize(m_Renderer->Window(), 854, 480);
        glViewport(0, 0, 854, 480);
    } else if (e.EntityName == "FullScreen") {
        printf("No fullscreen for now");
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
        auto menus = m_World->GetComponents("Menu");
        if (menus == nullptr) {
            return 0;
        }

        if (m_OpenSubMenu == EntityWrapper::Invalid) {
            //No submenu is open, open one.
            for (auto& menu : *menus) {
                EntityWrapper menuEntity = EntityWrapper(m_World, menu.EntityID);
                auto serverListSpawner = menuEntity.FirstChildByName(e.Command + "Spawner");
                if (!serverListSpawner.HasComponent("Spawner")) {
                    return 0;
                }
                m_OpenSubMenu = SpawnerSystem::Spawn(serverListSpawner, serverListSpawner);
                break;
            }

        } else if(!m_OpenSubMenu.HasComponent("ServerList")) {
            //Menu is open, but not the right one, delete the old one and open a new one.
            m_World->DeleteEntity(m_OpenSubMenu.ID);
            m_OpenSubMenu = EntityWrapper::Invalid;

            for (auto& menu : *menus) {
                EntityWrapper menuEntity = EntityWrapper(m_World, menu.EntityID);
                auto serverListSpawner = menuEntity.FirstChildByName(e.Command + "Spawner");
                if (!serverListSpawner.HasComponent("Spawner")) {
                    return 0;
                }
                m_OpenSubMenu = SpawnerSystem::Spawn(serverListSpawner, serverListSpawner);
            }


        } else {
            //Serverlist submenu is open, close it.
            printf("\n\nMenuShit\n\n");
            m_World->DeleteEntity(m_OpenSubMenu.ID);
            m_OpenSubMenu = EntityWrapper::Invalid;
        }
    }
    return true;
}