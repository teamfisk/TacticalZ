#include <imgui/imgui.h>
#include <glm/gtx/common.hpp>
#include <boost/filesystem/path.hpp>
#include <nativefiledialog/nfd.h>
#include "../Core/System.h"
#include "../Core/EMousePress.h"
#include "../Core/EMouseRelease.h"
#include "../Core/EMouseMove.h"
#include "../Core/ConfigFile.h"
#include "../Input/EInputCommand.h"
#include "../Rendering/IRenderer.h"
#include "../Core/Transform.h"
#include "../Core/EFileDropped.h"
#include "../Core/EntityFilePreprocessor.h"
#include "../Core/EntityFileParser.h"
#include "../Core/EntityFileWriter.h"

class EditorSystemOld : public ImpureSystem
{
public:
    EditorSystemOld(World* world, EventBroker* eventBroker, IRenderer* renderer);

    virtual void Update(double dt) override;

private:
    IRenderer* m_Renderer;
    Camera* m_Camera = nullptr;

    bool m_Enabled;
    bool m_Visible;
    boost::filesystem::path m_DefaultEntityDir;
    boost::filesystem::path m_CurrentFile;
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

    EntityID m_Widget = EntityID_Invalid;
    EntityID m_WidgetX = EntityID_Invalid;
    EntityID m_WidgetPlaneX = EntityID_Invalid;
    EntityID m_WidgetY = EntityID_Invalid;
    EntityID m_WidgetPlaneY = EntityID_Invalid;
    EntityID m_WidgetZ = EntityID_Invalid;
    EntityID m_WidgetPlaneZ = EntityID_Invalid;
    EntityID m_WidgetOrigin = EntityID_Invalid;
    glm::vec3 m_WidgetCurrentAxis;
    float m_WidgetPickingDepth = 0.f;
    glm::vec3 m_WidgetPickingPosition = glm::vec3(0);

    EntityID m_Selection = EntityID_Invalid;
    EntityID m_LastSelection = EntityID_Invalid;
    EntityID m_UIDraggingEntity = EntityID_Invalid;
    glm::vec3 m_Position;
    std::string m_LastDroppedFile;

    static boost::filesystem::path openDialog(boost::filesystem::path defaultPath);
    static boost::filesystem::path saveDialog(boost::filesystem::path defaultPath);

    EventRelay<EditorSystemOld, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<EditorSystemOld, Events::MouseRelease> m_EMouseRelease;
    bool OnMouseRelease(const Events::MouseRelease& e);
    EventRelay<EditorSystemOld, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
    EventRelay<EditorSystemOld, Events::MouseMove> m_EMouseMove;
    bool OnMouseMove(const Events::MouseMove& e);
    EventRelay<EditorSystemOld, Events::FileDropped> m_EFileDropped;
    bool OnFileDropped(const Events::FileDropped& e);
    
    void Picking();
    void createWidget();
    void updateWidget();
    void setWidgetMode(WidgetMode newMode);
    void setWidgetSpace(WidgetSpace space);
    void drawUI(World* world, double dt);
    bool createDeleteButton(std::string componentType);
    bool createEntityNode(World* world, EntityID entity);
    void changeParent(EntityID entity, EntityID newParent);
    void fileImport(World* world);
    void fileSave(World* world);
    void fileSaveAs(World* world);
};