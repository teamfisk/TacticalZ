#ifndef EditorCameraInputController_h__
#define EditorCameraInputController_h__

#include <imgui/imgui.h>
#include "../Input/FirstPersonInputController.h"
#include "../Core/EMousePress.h"
#include "../Core/EMouseRelease.h"
#include "../Core/EMouseScroll.h"
#include "../Core/ConfigFile.h"

template <typename EventContext>
class EditorCameraInputController : public FirstPersonInputController<EventContext>
{
public:
    EditorCameraInputController(EventBroker* eventBroker, unsigned int playerID, EntityWrapper playerEntity)
        : FirstPersonInputController(eventBroker, playerID, playerEntity)
    {
        EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &EditorCameraInputController::OnMousePress);
        EVENT_SUBSCRIBE_MEMBER(m_EMouseRelease, &EditorCameraInputController::OnMouseRelease);
        EVENT_SUBSCRIBE_MEMBER(m_EMouseScroll, &EditorCameraInputController::OnMouseScroll);

        m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
        m_SpeedMultiplier = m_Config->Get<float>("Editor.CameraSpeed", 3.f);
    }
    
    virtual const glm::vec3 Movement() const override { return m_Movement * static_cast<float>(m_SpeedMultiplier); }

    void Enable() { m_Enabled = true; }
    void Disable() { m_Enabled = false; }

    virtual bool OnCommand(const Events::InputCommand& e) override
    {
        if (glm::abs(e.Value) > 0 && !m_MouseLocked) {
            return false;
        }

        ImGuiIO& io = ImGui::GetIO();
        if (glm::abs(e.Value) > 0 && (io.WantCaptureKeyboard || io.WantCaptureMouse)) {
            return false;
        }

        if (e.Command == "Jump") {
            if (e.Value > 0) {
                m_Movement.y = glm::max(e.Value, 1.f);
            } else {
                m_Movement.y = 0.f;
            }
        }

        if (e.Command == "Crouch") {
            if (e.Value > 0) {
                m_Movement.y = glm::min(-e.Value, -1.f);
            } else {
                m_Movement.y = 0.f;
            }
        }

        if (e.Command == "Sprint") {
            if (e.Value > 0) {
                m_SpeedMultiplier *= 2.f;
            } else {
                m_SpeedMultiplier /= 2.f;
            }
        }

        return FirstPersonInputController::OnCommand(e);
    }

protected:
    ConfigFile* m_Config;
    bool m_Enabled = false;
    double m_SpeedMultiplier = 1.f;

    EventRelay<EventContext, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e)
    {
        if (!m_Enabled) {
            return false;
        }

        if (e.Button == GLFW_MOUSE_BUTTON_2) {
            ImGuiIO& io = ImGui::GetIO();
            if (!io.WantCaptureMouse) {
                LockMouse();
            }
        }
        return true;
    }
    EventRelay<EventContext, Events::MouseRelease> m_EMouseRelease;
    bool OnMouseRelease(const Events::MouseRelease& e)
    {
        if (!m_Enabled) {
            return false;
        }

        if (e.Button == GLFW_MOUSE_BUTTON_2) {
            UnlockMouse();
        }
        return true;
    }
    EventRelay<EventContext, Events::MouseScroll> m_EMouseScroll;
    bool OnMouseScroll(const Events::MouseScroll& e)
    {
        if (!m_Enabled) {
            return false;
        }
            
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
            return false;
        }

        m_SpeedMultiplier += e.DeltaY * (0.1 * m_SpeedMultiplier);
        m_Config->Set("Editor.CameraSpeed", m_SpeedMultiplier);
        m_Config->SaveToDisk();
        return true;
    }
};

#endif