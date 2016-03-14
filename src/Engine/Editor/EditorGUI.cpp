#include "Editor/EditorGUI.h"
#include "Rendering/ESetCamera.h"

EditorGUI::EditorGUI(World* world, EventBroker* eventBroker) 
    : m_World(world)
    , m_EventBroker(eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &EditorGUI::OnKeyDown);
    EVENT_SUBSCRIBE_MEMBER(m_EFileDropped, &EditorGUI::OnFileDropped);
    EVENT_SUBSCRIBE_MEMBER(m_EPause, &EditorGUI::OnPause);
    EVENT_SUBSCRIBE_MEMBER(m_EResume, &EditorGUI::OnResume);
    EVENT_SUBSCRIBE_MEMBER(m_ELockMouse, &EditorGUI::OnLockMouse);
    EVENT_SUBSCRIBE_MEMBER(m_EUnlockMouse, &EditorGUI::OnUnlockMouse);
}

void EditorGUI::Draw()
{
    ImGui::ShowTestWindow();
    drawMenu();
    drawTools();
    drawEntities(m_World);
    drawComponents(m_CurrentSelection);
    drawModals();
}

void EditorGUI::SelectEntity(EntityWrapper entity)
{
    m_CurrentSelection = entity;
    if (m_OnEntitySelected != nullptr) {
        m_OnEntitySelected(entity);
    }
}

void EditorGUI::drawMenu()
{

}

void EditorGUI::drawTools()
{
    if (!ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    // Widget modes
    createWidgetToolButton(WidgetMode::Translate);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Translate (W)");
    }
    ImGui::SameLine();
    createWidgetToolButton(WidgetMode::Rotate);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Rotate (E)");
    }
    ImGui::SameLine();
    createWidgetToolButton(WidgetMode::Scale);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Scale (R)");
    }

    ImGui::SameLine();
    ImGui::ItemSize(ImVec2(5, 0));

    // Widget space
    ImGui::SameLine();
    GLuint spaceTexture = 0;
    if (m_CurrentWidgetSpace == WidgetSpace::Global) {
        spaceTexture = tryLoadTexture("Textures/Icons/Global.png");
    } else if (m_CurrentWidgetSpace == WidgetSpace::Local) {
        spaceTexture = tryLoadTexture("Textures/Icons/Local.png");
    }
    if (ImGui::ImageButton(reinterpret_cast<void*>(spaceTexture), ImVec2(24, 24), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
        toggleWidgetSpace();
    }
    if (ImGui::IsItemHovered()) {
        if (m_CurrentWidgetSpace == WidgetSpace::Global) {
            ImGui::SetTooltip("Widget space: Global (X)");
        } else if (m_CurrentWidgetSpace == WidgetSpace::Local) {
            ImGui::SetTooltip("Widget space: Local (X)");
        }
    }

    ImGui::SameLine();
    ImGui::ItemSize(ImVec2(5, 0));

    // Play button
    ImGui::SameLine();
    if (ImGui::ImageButton(reinterpret_cast<void*>(tryLoadTexture("Textures/Icons/Play.png")), ImVec2(24, 24), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), (!m_Paused) ? ImVec4(0, 1, 0, 1) : ImVec4(1, 1, 1, 1))) {
        Events::Resume e;
        e.World = m_World;
        m_EventBroker->Publish(e);
    }
    // Pause button
    ImGui::SameLine();
    if (ImGui::ImageButton(reinterpret_cast<void*>(tryLoadTexture("Textures/Icons/Pause.png")), ImVec2(24, 24), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), (m_Paused) ? ImVec4(0, 1, 0, 1) : ImVec4(1, 1, 1, 1))) {
        Events::Pause e;
        e.World = m_World;
        m_EventBroker->Publish(e);
    }

    ImGui::End();
}

void EditorGUI::drawEntities(World* world)
{
    if (!ImGui::Begin("Entities")) {
        ImGui::End();
        return;
    }

    float buttonWidth = (ImGui::GetContentRegionAvailWidth() - 10.f) / 3.f ;
    if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
        entityCreate(world, m_CurrentSelection);
    }
    ImGui::SameLine(0.f, 5.f);
    if (ImGui::Button("Import", ImVec2(buttonWidth, 0))) {
        entityImport(world);
    }
    ImGui::SameLine(0.f, 5.f);
    ImGui::ButtonEx("Reference", ImVec2(buttonWidth, 0), ImGuiButtonFlags_Disabled);

    // Naming
    char buffer[256];
    buffer[0] = '\0';
    buffer[255] = '\0';
    std::size_t nameLength = 0;
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll;
    if (m_CurrentSelection.Valid()) {
        std::string name = world->GetName(m_CurrentSelection.ID);
        nameLength = name.length();
        if (!name.empty()) {
            memcpy(buffer, name.c_str(), std::min(sizeof(buffer) - 1, name.length() + 1));
        }
    } else {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }
    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 7.f);
    if (ImGui::InputText("", &buffer[0], sizeof(buffer), flags)) {
        if (m_CurrentSelection.Valid()) {
            if (m_OnEntityChangeName != nullptr) {
                m_OnEntityChangeName(m_CurrentSelection, std::string(buffer));
                SetDirty(m_CurrentSelection);
            }
        }
    }
    ImGui::PopItemWidth();

    ImGui::ItemSize(ImVec2(0, 3));

    drawEntitiesRecursive(world, EntityID_Invalid);

    ImGui::End();
}

void EditorGUI::drawEntitiesRecursive(World* world, EntityID parent)
{
    auto entityChildren = world->GetEntityChildren();
    auto range = entityChildren.equal_range(parent);
    for (auto it = range.first; it != range.second; it++) {
        if (drawEntityNode(EntityWrapper(world, it->second))) {
            drawEntitiesRecursive(world, it->second);
            ImGui::TreePop();
        }
    }
}

bool EditorGUI::drawEntityNode(EntityWrapper entity)
{
    // Custom button hitbox to select entities on top of tree node
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvailWidth();
    ImRect bb(pos + ImVec2(20, 0), pos + ImVec2(width, 14));
    auto window = ImGui::GetCurrentWindow();
    if (m_CurrentSelection == entity) {
        const ImU32 col = window->Color(ImGuiCol_HeaderActive);
        window->DrawList->AddRectFilled(bb.Min, bb.Max, col);
    }
    ImGuiID id = window->GetID((std::string("#SelectButton") + std::to_string(entity.ID)).c_str());
    bool hovered = false;
    bool held = false;
    if (ImGui::ButtonBehavior(bb, id, &hovered, &held)) {
        SelectEntity(entity);
    }
    // Handle entity dragging
    if (held) {
        ImVec2 entityDragDelta = ImGui::GetMouseDragDelta(0);
        if (std::abs(entityDragDelta.x) > 0 && std::abs(entityDragDelta.y) > 0) {
            if (m_CurrentlyDragging == EntityWrapper::Invalid) {
                m_CurrentlyDragging = entity;
                LOG_DEBUG("Started dragging %i", entity.ID);
            }
            ImGui::SetNextWindowPos(ImGui::GetIO().MousePos + ImVec2(20, 0));
            ImGui::Begin("Change parent", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
            ImGui::Text(formatEntityName(entity).c_str());
            ImGui::End();
        }
    }
    // Entity context menu
    std::string contextMenuUniqueID = std::string("EntityContextMenu") + std::to_string(entity.ID);
    if (hovered && ImGui::IsMouseClicked(1)) {
        ImGui::OpenPopup(contextMenuUniqueID.c_str());
    }
    if (ImGui::BeginPopup(contextMenuUniqueID.c_str())) {
        ImGui::TextDisabled(formatEntityName(entity).c_str());
        if (ImGui::MenuItem("Save", "Ctrl+S")) { 
            entitySave(entity);
        } else 
        if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) { 
            entitySave(entity, true);
        } else 
        if (ImGui::MenuItem("Delete", "Del")) {
            entityDelete(entity);
        } else 
        if (ImGui::MenuItem("Move to root")) {
            entityChangeParent(entity, EntityWrapper::Invalid);
        }
        ImGui::EndPopup();
    }

    ImGui::PushID(("EntityNode" + std::to_string(entity.ID)).c_str());
    ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_Once);
    if (ImGui::TreeNode(formatEntityName(entity).c_str())) {
        // Handle drop events for reparenting
        if (m_CurrentlyDragging != EntityWrapper::Invalid && ImGui::IsItemHoveredRect() && ImGui::IsMouseReleased(0)) {
            entityChangeParent(m_CurrentlyDragging, entity);
            m_CurrentlyDragging = EntityWrapper::Invalid;
        }
        ImGui::PopID();
        return true;
    } else {
        ImGui::PopID();
        return false;
    }
}

void EditorGUI::drawComponents(EntityWrapper entity)
{
    std::stringstream title;
    title << "Components";
    if (entity.Valid()) {
        title << " " << formatEntityName(entity);
    }
    title << "###Components";
    if (!ImGui::Begin(title.str().c_str())) {
        ImGui::End();
        return;
    }

    if (!entity.Valid()) {
        ImGui::End();
        return;
    }

    auto& pools = entity.World->GetComponentPools();
    // Create list of component types available to be added
    std::vector<const char*> componentTypes;
    for (auto& pair : pools) {
        // Don't list components the entity already has attached
        if (!entity.HasComponent(pair.first)) {
            componentTypes.push_back(pair.first.c_str());
        }
    }
    // Sort components in alphabetical order
    std::sort(componentTypes.begin(), componentTypes.end(), compareCharArray);
    // Draw combo box
    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 10.f);
    int selectedItem = -1;
    if (ImGui::Combo("", &selectedItem, componentTypes.data(), static_cast<int>(componentTypes.size()), static_cast<int>(componentTypes.size()))) {
        if (selectedItem != -1) {
            if (m_OnComponentAttach != nullptr) {
                std::string chosenComponentType(componentTypes.at(selectedItem));
                m_OnComponentAttach(entity, chosenComponentType);
                SetDirty(entity);
            }
        }
    }
    ImGui::PopItemWidth();

    for (auto& pair : pools) {
        const std::string& componentType = pair.first;
        auto pool = pair.second;
        // Don't show components the entity doesn't have attached
        if (!entity.HasComponent(componentType)) {
            continue;
        }
        // Handle deletion with early out
        if (createDeleteButton(componentType)) {
            if (m_OnComponentDelete != nullptr) {
                m_OnComponentDelete(entity, componentType);
                continue;
            }
        }
        // Draw the actual component node
        drawComponentNode(entity, pool->ComponentInfo());
    }

    ImGui::End();
}

bool EditorGUI::drawComponentNode(EntityWrapper entity, const ComponentInfo& ci)
{
    if (!ImGui::CollapsingHeader(ci.Name.c_str(), nullptr, true, true)) {
        return false;
    }

    // Show component annotation
    const std::string annotation = ci.Meta->Annotation;
    if (!annotation.empty()) {
        ImGui::TextWrapped(annotation.c_str());
    }

    // Draw component fields
    ComponentWrapper& component = entity.World->GetComponent(entity.ID, ci.Name);
    for (auto& fieldName : ci.FieldsInOrder) {
        const ComponentInfo::Field_t& field = ci.Fields.at(fieldName);

        // Draw the field widget based on its type
        bool dirty = drawComponentField(component, field);
        if (dirty) {
            SetDirty(entity);
        }
        ImGui::SameLine();
        // Draw field name
        ImGui::Text(fieldName.c_str());
        // Draw potential field annotation
        auto fieldAnnotationIt = ci.Meta->FieldAnnotations.find(fieldName);
        if (fieldAnnotationIt != ci.Meta->FieldAnnotations.end()) {
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(fieldAnnotationIt->second.c_str());
            }
        }
    }

    if (ci.Name == "Camera") {
        if (ImGui::Button("Activate")) {
            Events::SetCamera e;
            e.CameraEntity = entity;
            m_EventBroker->Publish(e);
        }
    }

    if (ci.Name == "Spawner") {
        if (ImGui::Button("Activate")) {
            Events::SpawnerSpawn e;
            e.Spawner = entity;
            e.Parent = entity;
            m_EventBroker->Publish(e);
        }
    }

    return true;
}

bool EditorGUI::drawComponentField(ComponentWrapper& c, const ComponentInfo::Field_t& field)
{
    // Push an unique widget id so different components with fields with equal names are still counted as different
    ImGui::PushID((c.Info.Name + field.Name).c_str());

    bool dirty = false;

    if (field.Type == "Vector") {
        dirty = drawComponentField_Vector(c, field);
    } else if (field.Type == "Color") {
        dirty = drawComponentField_Color(c, field);
    //} else if (field.Type == "Quaternion") {
    } else if (field.Type == "int") {
        dirty = drawComponentField_int(c, field);
    } else if (field.Type == "enum") {
        dirty = drawComponentField_enum(c, field);
    } else if (field.Type == "float") {
        dirty = drawComponentField_float(c, field);
    } else if (field.Type == "double") {
        dirty = drawComponentField_double(c, field);
    } else if (field.Type == "bool") {
        dirty = drawComponentField_bool(c, field);
    } else if (field.Type == "string") {
        dirty = drawComponentField_string(c, field);
    } else {
        ImGui::TextDisabled(field.Type.c_str());
    }

    if (dirty) {
        c[field.Name].SetAllDirty();
    }

    ImGui::PopID();

    return dirty;
}

bool EditorGUI::drawComponentField_Vector(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<glm::vec3>(field.Name);
    if (field.Name == "Scale") {
        // Limit scale values to a minimum of 0
        return ImGui::DragFloat3("", glm::value_ptr(val), 0.1f, 0.f, std::numeric_limits<float>::max());
    } else if (field.Name == "Orientation") {
        //glm::vec3 tempVal = val;
        glm::vec3 originalVal = val;

        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        glm::tvec3<bool> isSnapping(false, false, false);
        bool changed = ImGui::DragFloat3("", glm::value_ptr(val), 0.066666f);
        if (changed) {
            // Make orentations have a period of 2*Pi
            val = glm::fmod(val, glm::vec3(glm::two_pi<float>()));
            for (int i = 0; i < 3; i++) {
                if (val[i] < 0) {
                    val[i] += glm::two_pi<float>();
                }
            }
        }

        // Snap to angle
        //float snapRange = glm::pi<float>() / 15.f;
        //float snapAngle = glm::quarter_pi<float>();
        //glm::vec3 snap = glm::fmod(val, glm::vec3(snapAngle));
        //for (int i = 0; i < 3; i++) {
        //    isSnapping[i] = glm::abs(snap[i] - (snapRange / 2.f)) < snapRange;
        //}
        //if (changed && ImGui::IsMouseDown(0)) {
        //    glm::vec3 change = val - originalVal;
        //    for (int i = 0; i < 3; i++) {
        //        if (isSnapping[i] && glm::abs(change[i]) < snapRange) {
        //            val[i] -= snap[i] - snapRange;
        //        }
        //    }
        //}

        // Draw snapping outline
        float width = ImGui::CalcItemWidth() / 3.f;;
        float spacing = GImGui->Style.ItemInnerSpacing.x;
        for (int i = 0; i < 3; i++) {
            if (isSnapping[i]) {
                ImVec2 pos = cursorPos + ImVec2(i * (width + spacing), 0.f);
                ImRect bb(pos - ImVec2(1, 1), pos + ImVec2(width, 17));
                auto window = ImGui::GetCurrentWindow();
                const ImU32 col = window->Color(ImGuiCol_HeaderActive);
                window->DrawList->AddRect(bb.Min, bb.Max, col, 3.f);
            }
        }

        return changed;
    } else {
        return ImGui::DragFloat3("", glm::value_ptr(val), 0.1f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
    }
}

bool EditorGUI::drawComponentField_Color(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<glm::vec4>(field.Name);
    return ImGui::ColorEdit4("", glm::value_ptr(val), true);
}

bool EditorGUI::drawComponentField_int(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<int>(field.Name);
    return ImGui::InputInt("", &val);
}

bool EditorGUI::drawComponentField_enum(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto fieldEnumDefIt = c.Info.Meta->FieldEnumDefinitions.find(field.Name);
    if (fieldEnumDefIt == c.Info.Meta->FieldEnumDefinitions.end()) {
        return drawComponentField_int(c, field);
    }

    auto& val = c.Field<int>(field.Name);
    int selectedItem = -1;
    std::stringstream enumKeys;
    std::vector<int> enumValues;
    int i = 0;
    for (auto& kv : fieldEnumDefIt->second) {
        enumKeys << kv.first << " (" << kv.second << ")" << '\0';
        enumValues.push_back(kv.second);
        if (val == kv.second) {
            selectedItem = i;
        }
        i++;
    }
    if (ImGui::Combo("", &selectedItem, enumKeys.str().c_str())) {
        val = enumValues.at(selectedItem);
        return true;
    } else {
        return false;
    }
}

bool EditorGUI::drawComponentField_float(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<float>(field.Name);
    return ImGui::InputFloat("", &val, 0.01f, 1.f);
}

bool EditorGUI::drawComponentField_double(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    float tempVal = static_cast<float>(c.Field<double>(field.Name));
    if (ImGui::InputFloat("", &tempVal, 0.01f, 1.f)) {
        c.SetField(field.Name, static_cast<double>(tempVal));
        return true;
    } else {
        return false;
    }
}

bool EditorGUI::drawComponentField_bool(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<bool>(field.Name);
    return ImGui::Checkbox("", &val);
}

bool EditorGUI::drawComponentField_string(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    bool result = false;

    auto& val = c.Field<std::string>(field.Name);

    char tempString[1024]; // Let's just hope this is an sufficiently large buffer for strings :)
    tempString[1023] = '\0'; // Null terminator just in case the string is larger than the buffer
    // Copy the string into the buffer, taking the null terminator into account
    memcpy(tempString, val.c_str(), std::min(val.length() + 1, sizeof(tempString) - 1));
    if (ImGui::InputText("", tempString, sizeof(tempString))) {
        val = std::string(tempString);
        result = true;
    }

    // Handle file drag and drop
    if (ImGui::IsItemHovered() && !m_DroppedFile.empty()) {
        // Unset potential input focus or our newly set value will be overwritten!
        if (ImGui::IsItemActive()) {
            ImGui::SetActiveID(0, nullptr);
        }
        // Set the actual dropped value
        val = m_DroppedFile;
        m_DroppedFile = "";
    }
    
    return result;
}

void EditorGUI::drawModals()
{
    for (auto& modal : m_ModalsToOpen) {
        ImGui::OpenPopup(modal.c_str());
    }
    m_ModalsToOpen.clear();

    if (ImGui::BeginPopupModal("Import failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Entity import failed. Check console for more information.\n\n");
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvailWidth() - 120);
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Save failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Entity save failed on an exception.\nMessage: %s\n\n", m_LastErrorMessage.c_str());
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvailWidth() - 120);
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Confirm deletion", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (m_ModalData.count("Confirm deletion") == 0) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::Text("Are you sure you want to delete entity \"%s\"?", formatEntityName(m_CurrentSelection).c_str());
        ImGui::ItemSize(ImVec2(5.f, 0.f));
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvailWidth() - 2*60);
        if (ImGui::Button("Delete (Del)", ImVec2(60, 0))) {
            entityDelete(boost::any_cast<EntityWrapper>(m_ModalData.at("Confirm deletion")));
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(60, 0))) {
            m_ModalData.erase("Confirm deletion");
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::EndPopup();
    }
}

bool EditorGUI::createDeleteButton(const std::string& componentType)
{
    float width = ImGui::GetContentRegionAvailWidth();
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    auto pos = ImGui::GetCursorScreenPos() + ImVec2(width - 14.f, 1);
    ImRect bb = ImRect(pos, pos + ImVec2(14.f, 14.f));
    std::string idString = "#DELETE";
    idString += componentType;
    ImGuiID id = window->GetID(idString.c_str());
    bool hovered;
    bool held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    //ImU32 col = window->Color((held && hovered) ? ImGuiCol_CloseButtonActive : hovered ? ImGuiCol_CloseButtonHovered : ImGuiCol_CloseButton);
    ImU32 col = window->Color((held && hovered) ? ImGuiCol_CloseButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    window->DrawList->AddCircleFilled(bb.GetCenter(), 7.f, col, 16);
    return pressed;
}

void EditorGUI::createWidgetToolButton(WidgetMode mode)
{
    GLuint texture = 0;
    switch (mode) {
    case WidgetMode::Translate:
        texture = tryLoadTexture("Textures/Icons/Translate.png");
        break;
    case WidgetMode::Rotate:
        texture = tryLoadTexture("Textures/Icons/Rotate.png");
        break;
    case WidgetMode::Scale:
        texture = tryLoadTexture("Textures/Icons/Scale.png");
        break;
    }
    if (ImGui::ImageButton(
            reinterpret_cast<void*>(texture),
            ImVec2(24, 24), 
            ImVec2(0, 1), 
            ImVec2(1, 0), 
            -1, 
            ImVec4(0, 0, 0, 0), 
            (m_CurrentWidgetMode == mode) ? ImVec4(0, 1, 0, 1) : ImVec4(1, 1, 1, 1)
        )
    ) {
        setWidgetMode(mode);
    }
}

bool EditorGUI::OnKeyDown(const Events::KeyDown& e)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return false;
    }

    if (e.ModCtrl && e.KeyCode == GLFW_KEY_S) {
        if (m_CurrentSelection.Valid()) {
            EntityWrapper baseParent = m_CurrentSelection;
            while (baseParent.Parent().Valid()) {
                baseParent = baseParent.Parent();
            }
            entitySave(baseParent);
        }
    }

    if (e.ModCtrl && e.KeyCode == GLFW_KEY_N) {
        entityCreate(m_World, m_CurrentSelection);
    }

    if (e.ModCtrl && e.KeyCode == GLFW_KEY_O) {
        entityImport(m_World);
    }

    if (e.ModCtrl && e.KeyCode == GLFW_KEY_C) {
        m_CopyTarget = m_CurrentSelection;
    }

    if (e.ModCtrl && e.KeyCode == GLFW_KEY_V) {
        if (m_OnEntityPaste != nullptr) {
            EntityWrapper copy = m_OnEntityPaste(m_CopyTarget, m_CurrentSelection);
            if (copy != EntityWrapper::Invalid) {
                SelectEntity(copy);
            }
        }
    }

    if (e.KeyCode == GLFW_KEY_DELETE) {
        if (m_CurrentSelection.Valid()) {
            entityDelete(m_CurrentSelection);
        }
    }

    if (!m_MouseLocked) {
        if (e.KeyCode == GLFW_KEY_W) {
            setWidgetMode(WidgetMode::Translate);
        }
        if (e.KeyCode == GLFW_KEY_E) {
            setWidgetMode(WidgetMode::Rotate);
        }
        if (e.KeyCode == GLFW_KEY_R) {
            setWidgetMode(WidgetMode::Scale);
        }

        if (e.KeyCode == GLFW_KEY_X) {
            toggleWidgetSpace();
        }
    }

    return true;
}

bool EditorGUI::OnFileDropped(const Events::FileDropped& e)
{
    // Make a best effort to make the path relative to the working directory of the executable
    m_DroppedFile = boost::filesystem::path(e.Path).lexically_relative(boost::filesystem::current_path()).string();
    // Compensate for Windows retardedness
    std::replace(m_DroppedFile.begin(), m_DroppedFile.end(), '\\', '/');
    // Special case for when people drop from the asset folder instead of from the symlink to the asset folders in bin
    boost::algorithm::replace_first(m_DroppedFile, "../assets/", "");
    return true;
}

bool EditorGUI::OnPause(const Events::Pause& e)
{
    if (e.World == m_World) {
        m_Paused = true;
    }
    return true;
}

bool EditorGUI::OnResume(const Events::Resume& e)
{
    if (e.World == m_World) {
        m_Paused = false;
    }
    return true;
}

bool EditorGUI::OnLockMouse(const Events::LockMouse& e)
{
    m_MouseLocked = true;
    return true;
}

bool EditorGUI::OnUnlockMouse(const Events::UnlockMouse& e)
{
    m_MouseLocked = false;
    return true;
}

boost::filesystem::path EditorGUI::fileOpenDialog()
{
    namespace bfs = boost::filesystem;
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_OpenDialog("xml", bfs::absolute(m_DefaultEntityPath).string().c_str(), &outPath);

    if (result == NFD_ERROR) {
        LOG_ERROR("NFD Error: %s", NFD_GetError());
        return bfs::path();
    } else if (result == NFD_CANCEL) {
        return bfs::path();
    } else {
        return bfs::absolute(outPath);
    }
}

boost::filesystem::path EditorGUI::fileSaveDialog()
{
    namespace bfs = boost::filesystem;
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_SaveDialog("xml", bfs::absolute(m_DefaultEntityPath).string().c_str(), &outPath);

    if (result == NFD_ERROR) {
        LOG_ERROR("NFD Error: %s", NFD_GetError());
        return bfs::path();
    } else if (result == NFD_CANCEL) {
        return bfs::path();
    } else {
        return bfs::absolute(outPath);
    }
}

const std::string EditorGUI::formatEntityName(EntityWrapper entity)
{
    if (!entity.Valid()) {
        return "EntityID_Invalid";
    }

    std::stringstream name;

    std::string entityName = entity.World->GetName(entity.ID);
    if (!entityName.empty()) {
        name << entityName;
    } else {
        name << "#" << entity.ID;
    }

    if (m_EntityFiles.count(entity) == 1) {
        name << " (" << m_EntityFiles.at(entity).Path.filename().string() << ")";
        if (m_EntityFiles.at(entity).Dirty) {
            name << "*";
        }
    }

    return name.str();
}

GLuint EditorGUI::tryLoadTexture(std::string filePath)
{
    GLuint texture = 0;
    try {
        texture = ResourceManager::Load<Texture>(filePath)->m_Texture;
    } catch (const std::exception&) { }
    return texture;
}

void EditorGUI::openModal(const std::string& modal)
{
    m_ModalsToOpen.insert(modal);
}

void EditorGUI::setWidgetMode(WidgetMode mode)
{
    if (m_OnWidgetMode != nullptr) {
        m_OnWidgetMode(mode);
    }
    m_CurrentWidgetMode = mode;
}

void EditorGUI::toggleWidgetSpace()
{
    if (m_CurrentWidgetSpace == WidgetSpace::Global) {
        m_CurrentWidgetSpace = WidgetSpace::Local;
    } else if (m_CurrentWidgetSpace == WidgetSpace::Local) {
        m_CurrentWidgetSpace = WidgetSpace::Global;
    }

    if (m_OnWidgetSpace != nullptr) {
        m_OnWidgetSpace(m_CurrentWidgetSpace);
    }
}

bool EditorGUI::compareCharArray(const char* c1, const char* c2)
{
    return strcmp(c1, c2) < 0;
}

void EditorGUI::SetDirty(EntityWrapper entity)
{
    EntityWrapper baseParent = entity;
    while (baseParent.Parent().Valid()) {
        baseParent = baseParent.Parent();
    }
    if (m_EntityFiles.find(baseParent) != m_EntityFiles.end()) {
        m_EntityFiles.at(baseParent).Dirty = true;
    }
}

void EditorGUI::entityImport(World* world)
{
    boost::filesystem::path filePath = fileOpenDialog();
    if (filePath.empty()) {
        return;
    }

    EntityWrapper entity = m_OnEntityImport(EntityWrapper(world, EntityID_Invalid), filePath);
    if (entity.Valid()) {
        m_EntityFiles[entity].Path = filePath;
        SelectEntity(entity);
    } else {
        openModal("Import failed");
    }
}

void EditorGUI::entitySave(EntityWrapper entity, bool saveAs /* = false */)
{
    boost::filesystem::path filePath;
    if (!saveAs && m_EntityFiles.count(entity) == 1) {
        filePath = m_EntityFiles.at(entity).Path;
    } else {
        filePath = fileSaveDialog();
    }

    if (filePath.empty()) {
        return;
    }

    try {
        m_OnEntitySave(entity, filePath);
        m_EntityFiles[entity].Path = filePath;
        m_EntityFiles[entity].Dirty = false;
    } catch (const std::exception& e) {
        m_LastErrorMessage = e.what();
        openModal("Save failed");
    }
}

void EditorGUI::entityCreate(World* world, EntityWrapper parent)
{
    if (m_OnEntityCreate != nullptr) {
        // Create the new entity in the world we're drawing for
        if (parent.World == nullptr) {
            parent.World = world;
        }
        EntityWrapper newEntity = m_OnEntityCreate(parent);
        SelectEntity(newEntity);
    }
}

void EditorGUI::entityDelete(EntityWrapper entity)
{
    std::string modalName = "Confirm deletion";

    if (m_ModalData.count(modalName) == 0) {
        m_ModalData[modalName] = entity;
        openModal(modalName);
    } else {
        if (boost::any_cast<EntityWrapper>(m_ModalData[modalName]) == entity) {
            EntityWrapper parent = entity.Parent();
            if (m_OnEntityDelete != nullptr) {
                SetDirty(entity);
                m_OnEntityDelete(entity);
                m_EntityFiles.erase(entity);
            }
            if (!m_CurrentSelection.Valid()) {
                SelectEntity(parent);
            }
        }
        m_ModalData.erase(modalName);
    }
}

void EditorGUI::entityChangeParent(EntityWrapper entity, EntityWrapper parent)
{
    if (entity == parent || parent.IsChildOf(entity)) {
        return;
    }

    if (m_OnEntityChangeParent != nullptr) {
        SetDirty(entity);
        m_OnEntityChangeParent(entity, parent);
        LOG_DEBUG("Changed parent of %i to %i", entity.ID, parent.ID);
    }
}
