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

#include "../Core/EventBroker.h"
#include "../Core/World.h"
#include "../Core/EntityWrapper.h"

class EditorGUI
{
public:
    EditorGUI(EventBroker* eventBroker)
        : m_EventBroker(eventBroker)
    { }

    void Draw(World* world);

    void SelectEntity(EntityWrapper entity);

    // Called when an entity is selected in the entity tree
    typedef std::function<void(EntityWrapper)> OnEntitySelectedCallback_t;
    void SetEntitySelectedCallback(OnEntitySelectedCallback_t f) { m_OnEntitySelected = f; }
    // Called when the user means to import an entity file. 
    // Expects an EntityWrapper of the newly created entity in return.
    typedef std::function<EntityWrapper(boost::filesystem::path)> OnEntityImport_t;
    void SetEntityImportCallback(OnEntityImport_t f) { m_OnEntityImport = f; }
    // Called when the user means to save an entity to file.
    // Expects a bool indicating whether the save was successful or not in return.
    typedef std::function<bool(EntityWrapper, boost::filesystem::path)> OnEntitySave_t;
    void SetEntitySaveCallback(OnEntitySave_t f) { m_OnEntitySave = f; }
    // Called when the user means to create a new entity.
    // @param EntityWrapper The parent of the entity to be created
    // @return EntityWrapper The newly created entity
    typedef std::function<EntityWrapper(EntityWrapper)> OnEntityCreate_t;
    void SetEntityCreateCallback(OnEntityCreate_t f) { m_OnEntityCreate = f; }
    // Called when the user means to delete an entity.
    typedef std::function<void(EntityWrapper)> OnEntityDelete_t;
    void SetEntityCreateCallback(OnEntityDelete_t f) { m_OnEntityDelete = f; }
    // Called when the user means to attach a new component to an entity.
    typedef std::function<void(EntityWrapper, const std::string&)> OnComponentAttach_t;
    void SetComponentAttachCallback(OnComponentAttach_t f) { m_OnComponentAttach = f; }


private:
    EventBroker* m_EventBroker;

    // State variables
    EntityWrapper m_CurrentSelection = EntityWrapper::Invalid;

    // Callbacks
    OnEntitySelectedCallback_t m_OnEntitySelected = nullptr;
    OnEntityImport_t m_OnEntityImport = nullptr;
    OnEntitySave_t m_OnEntitySave = nullptr;
    OnEntityCreate_t m_OnEntityCreate = nullptr;
    OnEntityDelete_t m_OnEntityDelete = nullptr;
    OnComponentAttach_t m_OnComponentAttach = nullptr;
    
    void drawMenu();
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
};

#endif