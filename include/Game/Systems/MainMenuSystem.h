#ifndef MainMenuSystem_h__
#define MainMenuSystem_h__

#include "Core/System.h"
#include "Rendering/IRenderer.h"
#include "Core/ResourceManager.h"
#include "Core/Event.h"
#include "Core/SpawnerSystem.h"


#include "GUI/EButtonClicked.h"
#include "GUI/EButtonPressed.h"
#include "GUI/EButtonReleased.h"
#include "Input/EInputCommand.h"
#include "Network/ESearchForServers.h"
#include "Network/EConnectRequest.h"


class MainMenuSystem : public ImpureSystem
{
public:
    MainMenuSystem(SystemParams params, IRenderer* renderer);
    virtual void Update(double dt) override;

private:
    IRenderer* m_Renderer;

    EventRelay<MainMenuSystem, Events::ButtonClicked> m_EClicked;
    bool OnButtonClick(const Events::ButtonClicked& e);
    EventRelay<MainMenuSystem, Events::ButtonReleased> m_EReleased;
    bool OnButtonRelease(const Events::ButtonReleased& e);
    EventRelay<MainMenuSystem, Events::ButtonPressed> m_EPressed;
    bool OnButtonPress(const Events::ButtonPressed& e);
    EventRelay<MainMenuSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);

    std::string m_CurrentCommand = "";
    EntityWrapper m_OpenSubMenu = EntityWrapper::Invalid;

};

#endif