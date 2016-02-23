#include "GUI/MainMenuSystem.h"

MainMenuSystem::MainMenuSystem(SystemParams params, IRenderer* renderer)
    : System(params)
    , ImpureSystem()
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPressed, &MainMenuSystem::OnButtonPress);
    EVENT_SUBSCRIBE_MEMBER(m_EReleased, &MainMenuSystem::OnButtonRelease);
    EVENT_SUBSCRIBE_MEMBER(m_EClicked, &MainMenuSystem::OnButtonClick);
    EVENT_SUBSCRIBE_MEMBER(m_EDisplayServerlist, &MainMenuSystem::OnDisplayServerlist);
}

void MainMenuSystem::Update(double dt)
{

}

bool MainMenuSystem::OnButtonClick(const Events::ButtonClicked& e)
{
    if(e.EntityName == "Play") {
        //Run play code
    } else if(e.EntityName == "Connecting") {
        LOG_INFO("Searching for LAN servers...");
        m_EventBroker->Publish(Events::SearchForServers());
    } else if(e.EntityName == "Host") {
        //Run host code
    } else if(e.EntityName == "Quit") {
        printf("No, you stay");
    } else if (e.EntityName == "Res1080") {
        glfwSetWindowSize(m_Renderer->Window(), 1920, 1080);
        printf("\n1080");
    } else if (e.EntityName == "Res720") {
        glfwSetWindowSize(m_Renderer->Window(), 1280, 720);
        glViewport(0, 0, 1280, 720);
        printf("\n720");
    } else if (e.EntityName == "Res480") {
        glfwSetWindowSize(m_Renderer->Window(), 854, 480);
        glViewport(0, 0, 854, 480);
        printf("\n480");
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

bool MainMenuSystem::OnDisplayServerlist(const Events::DisplayServerlist& e)
{
    LOG_INFO("This is a serverlist V2!");
    for (int i = 0; i < e.Serverlist.size(); i++) {
        LOG_INFO("%s:%i\t%s\t%i/8"
            , e.Serverlist[i].Address
            , e.Serverlist[i].Port
            , e.Serverlist[i].Name
            , e.Serverlist[i].PlayersConnected
        );
    }
    return true;
}

