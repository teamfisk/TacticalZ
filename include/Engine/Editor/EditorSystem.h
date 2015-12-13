#include <imgui/imgui.h>
#include "../Core/System.h"
#include "../Core/EMousePress.h"
#include "../Rendering/EPicking.h"
#include "../Rendering/RenderQueueFactory.h"

class EditorSystem : public ImpureSystem
{
public:
    EditorSystem(EventBroker* eventBroker);

    virtual void Update(World* world, double dt) override;

private:
    std::vector<glm::vec2> m_PickingQueue;
    EntityID m_Widget = 0;
    EntityID m_Selection = 0;
    EntityID m_LastSelection = 0;
    glm::vec3 m_Position;

    EventRelay<EditorSystem, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
    EventRelay<EditorSystem, Events::Picking> m_EPicking;
    bool OnPicking(const Events::Picking& e);

    void drawUI(World* world, double dt);
    bool createDeleteButton(std::string componentType);
};