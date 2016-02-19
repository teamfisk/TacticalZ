#include "GUI/ButtonSystem.h"

ButtonSystem::ButtonSystem(SystemParams params, IRenderer* renderer)
    : System(params)
    , PureSystem("Button")
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &ButtonSystem::OnMousePress);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseRelease, &ButtonSystem::OnMouseRelease);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseLock, &ButtonSystem::OnMouseLock);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseUnlock, &ButtonSystem::OnMouseUnlock);
}


bool ButtonSystem::OnMouseLock(const Events::LockMouse& e)
{
    m_MouseIsLocked = true;
    return true;
}


bool ButtonSystem::OnMouseUnlock(const Events::UnlockMouse& e)
{
    m_MouseIsLocked = false;
    return true;
}


bool ButtonSystem::OnMousePress(const Events::MousePress& e)
{
    if (e.Button == GLFW_MOUSE_BUTTON_1 && !m_MouseIsLocked) {
        m_PickData = m_Renderer->Pick(glm::vec2(e.X, e.Y));
        if (m_PickData.Entity != EntityID_Invalid && m_PickData.World == m_World) {
            if(m_World->HasComponent(m_PickData.Entity, "Button")) {
                //Entity is a button, save it and send pressed event.

                m_PickEntity = EntityWrapper(m_World, m_PickData.Entity);

                //You have clicked on a button entity, send pressed event.
                Events::ButtonPressed ePressed;
                ePressed.Entity = m_PickEntity;
                ePressed.EntityName = m_PickEntity.Name();
                m_EventBroker->Publish(ePressed);
            }
        }
    }
    return true;
}

bool ButtonSystem::OnMouseRelease(const Events::MouseRelease& e)
{
    if(!m_MouseIsLocked) {
        //Mouse is not locked, send release event.
        m_PickData = m_Renderer->Pick(glm::vec2(e.X, e.Y));
        if(m_PickData.Entity != EntityID_Invalid && m_PickData.World == m_World) {
            EntityWrapper ent = EntityWrapper(m_World, m_PickData.Entity);

            Events::ButtonReleased eReleased;
            eReleased.Entity = ent;
            m_EventBroker->Publish(eReleased);

            if(m_World->HasComponent(m_PickData.Entity, "Button")) {
                if (ent == m_PickEntity) {
                    //The entity you released the mouse button on is the same as you pressed it on. "Clicked"
                    Events::ButtonClicked eClicked;
                    eClicked.Entity = m_PickEntity;
                    eClicked.EntityName = m_PickEntity.Name();
                    m_EventBroker->Publish(eClicked);
                }
            }
        }
    }
    return true;
}

void ButtonSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cHealth, double dt)
{

}



   