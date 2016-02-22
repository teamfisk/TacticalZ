#ifndef MainMenuSystem_h__
#define MainMenuSystem_h__

#include "../Core/System.h"
#include "../Rendering/IRenderer.h"
#include "../Core/ResourceManager.h"
#include "../Core/Event.h"


#include "EButtonClicked.h"
#include "EButtonPressed.h"
#include "EButtonReleased.h"


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

};

#endif