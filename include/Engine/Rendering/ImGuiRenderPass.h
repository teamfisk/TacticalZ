#include <imgui/imgui.h>
#include "../OpenGL.h"
#include "IRenderer.h"
#include "../Core/EventBroker.h"
#include "../Core/EMousePress.h"
#include "../Core/EMouseRelease.h"
#include "../Core/EMouseMove.h"
#include "../Core/EKeyDown.h"
#include "../Core/EKeyUp.h"
#include "../Core/EKeyboardChar.h"

class ImGuiRenderPass
{
public:
    ImGuiRenderPass(IRenderer* renderer, EventBroker* eventBroker);

    void Update(double dt);
    void Draw();

private:
    IRenderer* m_Renderer;
    EventBroker* m_EventBroker;

    GLFWwindow* g_Window;
    double g_Time = 0.0;
    GLuint g_FontTexture;
    int g_ShaderHandle;
    int g_VertHandle;
    int g_FragHandle;
    int g_AttribLocationTex;
    int g_AttribLocationProjMtx;
    int g_AttribLocationPosition;
    int g_AttribLocationUV;
    int g_AttribLocationColor;
    GLuint g_VboHandle;
    GLuint g_VaoHandle;
    GLuint g_ElementsHandle;

    EventRelay<ImGuiRenderPass, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
    EventRelay<ImGuiRenderPass, Events::MouseRelease> m_EMouseRelease;
    bool OnMouseRelease(const Events::MouseRelease& e);
    EventRelay<ImGuiRenderPass, Events::MouseMove> m_EMouseMove;
    bool OnMouseMove(const Events::MouseMove& e);
    EventRelay<ImGuiRenderPass, Events::KeyDown> m_EKeyDown;
    bool OnKeyDown(const Events::KeyDown& e);
    EventRelay<ImGuiRenderPass, Events::KeyUp> m_EKeyUp;
    bool OnKeyUp(const Events::KeyUp& e);
    EventRelay<ImGuiRenderPass, Events::KeyboardChar> m_EKeyboardChar;
    bool OnKeyboardChar(const Events::KeyboardChar& e);

    bool createDeviceObjects();
    bool createFontsTexture();
};