#include "GUI/MainMenuSystem.h"

MainMenuSystem::MainMenuSystem(SystemParams params, IRenderer* renderer)
    : System(params)
    , ImpureSystem()
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPressed, &MainMenuSystem::OnButtonPress);
    EVENT_SUBSCRIBE_MEMBER(m_EReleased, &MainMenuSystem::OnButtonRelease);
    EVENT_SUBSCRIBE_MEMBER(m_EClicked, &MainMenuSystem::OnButtonClick);
}

void MainMenuSystem::Update(double dt)
{

}

bool MainMenuSystem::OnButtonClick(const Events::ButtonClicked& e)
{
    if(e.EntityName == "Play") {
        //Run play code
    } else if(e.EntityName == "Connect") {
        //Run connect code
    } else if(e.EntityName == "Host") {
        //Run host code
    } else if(e.EntityName == "Quit") {
        printf("No, you stay");
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

