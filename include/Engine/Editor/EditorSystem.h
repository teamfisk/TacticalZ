#include <imgui/imgui.h>
#include <glm/gtx/common.hpp>
#include "../Core/System.h"
#include "../Core/EMousePress.h"
#include "../Core/ConfigFile.h"
#include "../Input/EInputCommand.h"
#include "../Rendering/EPicking.h"
#include "../Rendering/RenderSystem.h"

class EditorSystem : public ImpureSystem
{
public:
    EditorSystem(EventBroker* eventBroker);

    virtual void Update(World* world, double dt) override;

private:
    bool m_Enabled;
    bool m_Visible;
    std::vector<glm::vec2> m_PickingQueue;
    EntityID m_Widget = 0;
    EntityID m_Selection = 0;
    EntityID m_LastSelection = 0;
    glm::vec3 m_Position;

    EventRelay<EditorSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<EditorSystem, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
    EventRelay<EditorSystem, Events::Picking> m_EPicking;
    bool OnPicking(const Events::Picking& e);

    void drawUI(World* world, double dt);
    bool createDeleteButton(std::string componentType);
};