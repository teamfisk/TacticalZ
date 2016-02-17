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
    } else if (e.EntityName == "Res1080") {
        glfwSetWindowSize(m_Renderer->Window(), 1920, 1080);
        printf("1080");
    } else if (e.EntityName == "Res720") {
        glfwSetWindowSize(m_Renderer->Window(), 1280, 720);
        glViewport(0, 0, 1280, 720);
        printf("720");
    } else if (e.EntityName == "Res480") {
        glfwSetWindowSize(m_Renderer->Window(), 854, 480);
        glViewport(0, 0, 854, 480);
        printf("480");
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

