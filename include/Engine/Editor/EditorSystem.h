#include <imgui/imgui.h>
#include <glm/gtx/common.hpp>
#include "../Core/System.h"
#include "../Core/EMousePress.h"
#include "../Core/EMouseRelease.h"
#include "../Core/EMouseMove.h"
#include "../Core/ConfigFile.h"
#include "../Input/EInputCommand.h"
#include "../Rendering/IRenderer.h"
#include "../Rendering/EPicking.h"
#include "../Rendering/RenderQueueFactory.h"

class EditorSystem : public ImpureSystem
{
public:
    EditorSystem(EventBroker* eventBroker, IRenderer* renderer);

    virtual void Update(World* world, double dt) override;

private:
    IRenderer* m_Renderer;
    World* m_World = nullptr;

    bool m_Enabled;
    bool m_Visible;
    std::vector<glm::vec2> m_PickingQueue;

    enum class WidgetMode
    {
        None,
        Translate,
        Rotate,
        Scale
    } m_WidgetMode = WidgetMode::None;

    enum class WidgetSpace
    {
        Local,
        Global
    } m_WidgetSpace = WidgetSpace::Global;

    EntityID m_Widget = 0;
    EntityID m_WidgetX = 0;
    EntityID m_WidgetY = 0;
    EntityID m_WidgetZ = 0;
    EntityID m_WidgetOrigin = 0;
    glm::vec3 m_WidgetCurrentAxis;
    float m_WidgetPickingDepth = 0.f;

    EntityID m_Selection = 0;
    EntityID m_LastSelection = 0;
    glm::vec3 m_Position;

    EventRelay<EditorSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<EditorSystem, Events::MouseRelease> m_EMouseRelease;
    bool OnMouseRelease(const Events::MouseRelease& e);
    EventRelay<EditorSystem, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
    EventRelay<EditorSystem, Events::MouseMove> m_EMouseMove;
    bool OnMouseMove(const Events::MouseMove& e);
    EventRelay<EditorSystem, Events::Picking> m_EPicking;
    bool OnPicking(const Events::Picking& e);

    void updateWidget();
    void setWidgetMode(WidgetMode newMode);
    void setWidgetSpace(WidgetSpace space);
    void drawUI(World* world, double dt);
    bool createDeleteButton(std::string componentType);
};