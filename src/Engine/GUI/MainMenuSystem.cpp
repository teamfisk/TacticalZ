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
    printf("\nClicked: %s", e.EntityName);
    return true;
}

bool MainMenuSystem::OnButtonRelease(const Events::ButtonReleased& e)
{
    printf("\nReleased");

    return true;
}

bool MainMenuSystem::OnButtonPress(const Events::ButtonPressed& e)
{
    printf("\nPressed: %s", e.EntityName);

    return true;
}

