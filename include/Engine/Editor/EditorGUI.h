#ifndef EditorGUI_h__
#define EditorGUI_h__

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <nativefiledialog/nfd.h>
#include <boost/filesystem.hpp>
#include "../Common.h"
#include "../GLM.h"
#include <glm/gtx/common.hpp>

#include "EditorWidgetSystem.h"
#include "../Core/EventBroker.h"
#include "../Core/World.h"
#include "../Core/EntityWrapper.h"
#include "../Core/ResourceManager.h"
#include "../Core/EPause.h"
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

    void Draw();

    void SelectEntity(EntityWrapper entity);

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
    // Called when the user means to attach a new component to an entity.
    typedef std::function<void(EntityWrapper, const std::string&)> OnComponentAttach_t;
    void SetComponentAttachCallback(OnComponentAttach_t f) { m_OnComponentAttach = f; }
    // Called when the user means to delete a component off an entity.
    typedef std::function<void(EntityWrapper, const std::string&)> OnComponentDelete_t;
    void SetComponentDeleteCallback(OnComponentDelete_t f) { m_OnComponentDelete = f; }
    // Called when the user selects a widget mode.
    typedef std::function<void(WidgetMode)> OnWidgetMode_t;
    void SetWidgetModeCallback(OnWidgetMode_t f) { m_OnWidgetMode = f; }

private:
    World* m_World;
    EventBroker* m_EventBroker;

    // Config variables
    const boost::filesystem::path m_DefaultEntityPath = boost::filesystem::path("Schema") / boost::filesystem::path("Entities");

    // State variables
    EntityWrapper m_CurrentSelection = EntityWrapper::Invalid;
    std::unordered_map<EntityWrapper, boost::filesystem::path> m_EntityFiles;
    EntityWrapper m_CurrentlyDragging = EntityWrapper::Invalid;
    std::string m_LastErrorMessage;
    WidgetMode m_CurrentWidgetMode = WidgetMode::Translate;

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

    // Utility functions
    boost::filesystem::path fileOpenDialog();
    boost::filesystem::path fileSaveDialog();
    const std::string formatEntityName(EntityWrapper entity);
    GLuint tryLoadTexture(std::string filePath);

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
    void drawComponentField(ComponentWrapper& c, const ComponentInfo::Field_t& field);
    void drawComponentField_Vector(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    void drawComponentField_Color(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    void drawComponentField_int(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    void drawComponentField_enum(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    void drawComponentField_float(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    void drawComponentField_double(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    void drawComponentField_bool(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    void drawComponentField_string(ComponentWrapper &c, const ComponentInfo::Field_t &field);
    void drawModals();

    // Custom UI elements
    bool createDeleteButton(const std::string& componentType);
};

#endif