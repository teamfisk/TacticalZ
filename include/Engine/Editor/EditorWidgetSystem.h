#ifndef EditorWidgetSystem_h__
#define EditorWidgetSystem_h__

#include <imgui/imgui.h>
#include "../GLM.h"
#include "../Core/System.h"
#include "../Rendering/IRenderer.h"
#include "../Rendering/Util/ScreenCoords.h"
#include "../Core/EMouseMove.h"
#include "../Core/EMousePress.h"
#include "../Core/EMouseRelease.h"

namespace Events
{

struct WidgetDelta : Event
{
    glm::vec3 Translation;
    glm::vec3 Rotation;
    glm::vec3 Scale;
};

}

class EditorWidgetSystem : public ImpureSystem, PureSystem
{
public:
    EditorWidgetSystem(SystemParams params, IRenderer* renderer);

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cEditorWidget, double dt) override;

private:
    IRenderer* m_Renderer;

    // State
    EntityWrapper m_PickEntity = EntityWrapper::Invalid;
    PickData m_PickData;
    glm::vec2 m_MouseDelta;

    EventRelay<EditorWidgetSystem, Events::MouseMove> m_EMouseMove;
    bool OnMouseMove(const Events::MouseMove& e);
    EventRelay<EditorWidgetSystem, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
    EventRelay<EditorWidgetSystem, Events::MouseRelease> m_EMouseRelease;
    bool OnMouseRelease(const Events::MouseRelease& e);
};

#endif
