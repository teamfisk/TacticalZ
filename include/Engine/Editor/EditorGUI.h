#ifndef EditorGUI_h__
#define EditorGUI_h__

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <nativefiledialog/nfd.h>
#include <boost/filesystem.hpp>
#include <boost/any.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "../Common.h"
#include "../GLM.h"
#include <glm/gtx/common.hpp>

#include "EditorWidgetSystem.h"
#include "../Core/EventBroker.h"
#include "../Core/World.h"
#include "../Core/EntityWrapper.h"
#include "../Core/ResourceManager.h"
#include "../Core/EPause.h"
#include "../Core/EKeyDown.h"
#include "../Core/ELockMouse.h"
#include "../Core/EFileDropped.h"
#include "../Rendering/Texture.h"

class EditorGUI
{
public:
    EditorGUI(World* world, EventBroker* eventBroker);

    enum class WidgetMode
    {
        Translate,
        Rotate,
        Scale
    };

    enum class WidgetSpace
    {
        Global,
        Local
    };

    void Draw();

    void SelectEntity(EntityWrapper entity);
    void SetDirty(EntityWrapper entity);

    // Called when an entity is selected in the entity tree
    typedef std::function<void(EntityWrapper)> OnEntitySelectedCallback_t;
    void SetEntitySelectedCallback(OnEntitySelectedCallback_t f) { m_OnEntitySelected = f; }
    // Called when the user means to import an entity file. 
    // @param EntityWrapper The entity to parent the imported entity to. The entity will be imported into the world of this entity.
    // @param boost::filesystem::path The path to the entity to import
    // @return EntityWrapper The newly created entity
    typedef std::function<EntityWrapper(EntityWrapper, boost::filesystem::path)> OnEntityImport_t;
    void SetEntityImportCallback(OnEntityImport_t f) { m_OnEntityImport = f; }
    // Called when the user means to save an entity to file.
    // Permitted to throw exceptions on save failure.
    typedef std::function<void(EntityWrapper, boost::filesystem::path)> OnEntitySave_t;
    void SetEntitySaveCallback(OnEntitySave_t f) { m_OnEntitySave = f; }
    // Called when the user means to create a new entity.
    // @param EntityWrapper The parent of the entity to be created
    // @return EntityWrapper The newly created entity
    typedef std::function<EntityWrapper(EntityWrapper)> OnEntityCreate_t;
    void SetEntityCreateCallback(OnEntityCreate_t f) { m_OnEntityCreate = f; }
    // Called when the user means to delete an entity.
    typedef std::function<void(EntityWrapper)> OnEntityDelete_t;
    void SetEntityDeleteCallback(OnEntityDelete_t f) { m_OnEntityDelete = f; }
    // Called when the user means to change the parent of an entity.
    typedef std::function<void(EntityWrapper, EntityWrapper)> OnEntityChangeParent_t;
    void SetEntityChangeParentCallback(OnEntityChangeParent_t f) { m_OnEntityChangeParent = f; }
    // Called when the user means to rename an entity.
    typedef std::function<void(EntityWrapper, const std::string&)> OnEntityChangeName_t;
    void SetEntityChangeNameCallback(OnEntityChangeName_t f) { m_OnEntityChangeName = f; }
    // Called when the user pastes an entity previously "copied"
    // @param EntityWrapper The entity to copy
    // @param EntityWrapper The entity to parent the new copy to
    // @return The new copy of the entity
    typedef std::function<EntityWrapper(EntityWrapper, EntityWrapper)> OnEntityPaste_t;
    void SetEntityPasteCallback(OnEntityPaste_t f) { m_OnEntityPaste = f; }
    // Called when the user means to attach a new component to an entity.
    typedef std::function<void(EntityWrapper, const std::string&)> OnComponentAttach_t;
    void SetComponentAttachCallback(OnComponentAttach_t f) { m_OnComponentAttach = f; }
    // Called when the user means to delete a component off an entity.
    typedef std::function<void(EntityWrapper, const std::string&)> OnComponentDelete_t;
    void SetComponentDeleteCallback(OnComponentDelete_t f) { m_OnComponentDelete = f; }
    // Called when the user selects a widget mode.
    typedef std::function<void(WidgetMode)> OnWidgetMode_t;
    void SetWidgetModeCallback(OnWidgetMode_t f) { m_OnWidgetMode = f; }
    // Called when the user selects a widget space.
    typedef std::function<void(WidgetSpace)> OnWidgetSpace_t;
    void SetWidgetSpaceCallback(OnWidgetSpace_t f) { m_OnWidgetSpace = f; }
    // Called when anything is modified making the world dirty
    // @param EntityWrapper The entity that was changed and marked as dirty
    typedef std::function<void(EntityWrapper)> OnDirty_t;
    void SetDirtyCallback(OnDirty_t f) { m_OnDirty = f; }
    // Called when the user wishes to undo
    typedef std::function<void()> OnUndo_t;
    void SetUndoCallback(OnUndo_t f) { m_OnUndo = f; }
private:
    World* m_World;
    EventBroker* m_EventBroker;

    struct EntityFileInfo
    {
        boost::filesystem::path Path;
        bool Dirty = false;
    };

    // Config variables
    const boost::filesystem::path m_DefaultEntityPath = boost::filesystem::path("Schema") / boost::filesystem::path("Entities");

    // State
    EntityWrapper m_CurrentSelection = EntityWrapper::Invalid;
    std::unordered_map<EntityWrapper, EntityFileInfo> m_EntityFiles;
    EntityWrapper m_CurrentlyDragging = EntityWrapper::Invalid;
    std::string m_LastErrorMessage;
    WidgetMode m_CurrentWidgetMode = WidgetMode::Translate;
    WidgetSpace m_CurrentWidgetSpace = WidgetSpace::Global;
    std::set<std::string> m_ModalsToOpen;
    std::map<std::string, boost::any> m_ModalData;
    std::string m_DroppedFile = "";
    bool m_Paused = false;
    bool m_MouseLocked = false;
    EntityWrapper m_CopyTarget = EntityWrapper::Invalid;

    // Callbacks
    OnEntitySelectedCallback_t m_OnEntitySelected = nullptr;
    OnEntityImport_t m_OnEntityImport = nullptr;
    OnEntitySave_t m_OnEntitySave = nullptr;
    OnEntityCreate_t m_OnEntityCreate = nullptr;
    OnEntityDelete_t m_OnEntityDelete = nullptr;
    OnEntityChangeParent_t m_OnEntityChangeParent = nullptr;
    OnEntityChangeName_t m_OnEntityChangeName = nullptr;
    OnComponentAttach_t m_OnComponentAttach = nullptr;
    OnComponentDelete_t m_OnComponentDelete = nullptr;
    OnWidgetMode_t m_OnWidgetMode = nullptr;
    OnWidgetSpace_t m_OnWidgetSpace = nullptr;
    OnEntityPaste_t m_OnEntityPaste = nullptr;
    OnDirty_t m_OnDirty = nullptr;
    OnUndo_t m_OnUndo = nullptr;

    // Events
    EventRelay<EditorGUI, Events::KeyDown> m_EKeyDown;
    bool OnKeyDown(const Events::KeyDown& e);
    EventRelay<EditorGUI, Events::FileDropped> m_EFileDropped;
    bool OnFileDropped(const Events::FileDropped& e);
    EventRelay<EditorGUI, Events::Pause> m_EPause;
    bool OnPause(const Events::Pause& e);
    EventRelay<EditorGUI, Events::Resume> m_EResume;
    bool OnResume(const Events::Resume& e);
    EventRelay<EditorGUI, Events::LockMouse> m_ELockMouse;
    bool OnLockMouse(const Events::LockMouse& e);
    EventRelay<EditorGUI, Events::UnlockMouse> m_EUnlockMouse;
    bool OnUnlockMouse(const Events::UnlockMouse& e);

    // Utility functions
    boost::filesystem::path fileOpenDialog();
    boost::filesystem::path fileSaveDialog();
    const std::string formatEntityName(EntityWrapper entity);
    GLuint tryLoadTexture(std::string filePath);
    void openModal(const std::string& modal);
    void setWidgetMode(WidgetMode mode);
    void toggleWidgetSpace();
    static bool compareCharArray(const char* c1, const char* c2);

    // Entity file handling methods
    void entityImport(World* world);
    void entitySave(EntityWrapper entity, bool saveAs = false);
    void entityCreate(World* world, EntityWrapper parent);
    void entityDelete(EntityWrapper entity);
    void entityChangeParent(EntityWrapper entity, EntityWrapper parent);
    
    // UI drawing methods
    void drawMenu();
    void drawTools();
    void drawEntities(World* world);
    void drawEntitiesRecursive(World* world, EntityID parent);
    bool drawEntityNode(EntityWrapper entity);
    void drawComponents(EntityWrapper entity);
    bool drawComponentNode(EntityWrapper entity, const ComponentInfo& componentType);
    bool drawComponentField(ComponentWrapper& c, const ComponentInfo::Field_t& field);
    bool drawComponentField_Vector(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    bool drawComponentField_Color(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    bool drawComponentField_int(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    bool drawComponentField_enum(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    bool drawComponentField_float(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    bool drawComponentField_double(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    bool drawComponentField_bool(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    bool drawComponentField_string(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    void drawModals();

    // Custom UI elements
    bool createDeleteButton(const std::string& componentType);
    void createWidgetToolButton(WidgetMode mode);
};

#endif