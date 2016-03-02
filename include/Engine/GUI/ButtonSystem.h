#ifndef ButtonSystem_h__
#define ButtonSystem_h__

#include "../Rendering/IRenderer.h"
#include "../Core/ConfigFile.h"
#include "../Rendering/PickingPass.h"
#include "../Core/ResourceManager.h"
#include "../Core/System.h"
#include "../Core/Event.h"
#include "../Core/EMousePress.h"
#include "../Core/EMouseRelease.h"
#include "../Core/ELockMouse.h"

#include "EButtonPressed.h"
#include "EButtonReleased.h"
#include "EButtonClicked.h"


class ButtonSystem : public PureSystem
{
public:
    ButtonSystem(SystemParams params, IRenderer* renderer);
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt) override;
private:
    IRenderer* m_Renderer;
    bool m_MouseIsLocked = false;

    EntityWrapper m_PickEntity = EntityWrapper::Invalid;
    PickData m_PickData;

    EventRelay<ButtonSystem, Events::LockMouse> m_EMouseLock;
    bool OnMouseLock(const Events::LockMouse& e);
    EventRelay<ButtonSystem, Events::UnlockMouse> m_EMouseUnlock;
    bool OnMouseUnlock(const Events::UnlockMouse& e);

    EventRelay<ButtonSystem, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
    EventRelay<ButtonSystem, Events::MouseRelease> m_EMouseRelease;
    bool OnMouseRelease(const Events::MouseRelease& e);
};



#endif