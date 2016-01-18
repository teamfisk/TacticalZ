#include "Editor/EditorGUI.h"

void EditorGUI::Draw(World* world)
{
    ImGui::ShowTestWindow();
    drawMenu();
    drawEntities(world);
    drawComponents(m_CurrentSelection);
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

void EditorGUI::drawEntities(World* world)
{
    if (!ImGui::Begin("Entities")) {
        return;
    }

    float buttonWidth = (ImGui::GetContentRegionAvailWidth() - 10.f) / 3.f ;
    ImGui::Button("Create", ImVec2(buttonWidth, 0));
    ImGui::SameLine(0.f, 5.f);
    ImGui::Button("Import", ImVec2(buttonWidth, 0));
    ImGui::SameLine(0.f, 5.f);
    ImGui::Button("Reference", ImVec2(buttonWidth, 0));

    ImGui::ItemSize(ImVec2(0, 3));

    drawEntitiesRecursive(world, EntityID_Invalid);

    ImGui::End();
}

void EditorGUI::drawEntitiesRecursive(World* world, EntityID parent)
{
    auto entityChildren = world->GetEntityChildren();
    auto range = entityChildren.equal_range(parent);
    for (auto it = range.first; it != range.second; it++) {
        if (EditorGUI::drawEntityNode(EntityWrapper(world, it->second))) {
            drawEntitiesRecursive(world, it->second);
            ImGui::TreePop();
        }
    }
}

bool EditorGUI::drawEntityNode(EntityWrapper entity)
{
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
    //if (held) {
    //    ImVec2 entityDragDelta = ImGui::GetMouseDragDelta(0);
    //    if (std::abs(entityDragDelta.x) > 0 && std::abs(entityDragDelta.y) > 0) {
    //        if (m_UIDraggingEntity == EntityID_Invalid) {
    //            m_UIDraggingEntity = entity;
    //            LOG_DEBUG("Started drag of entity %i", m_UIDraggingEntity);
    //        }
    //        ImGui::SetNextWindowPos(ImGui::GetIO().MousePos + ImVec2(20, 0));
    //        ImGui::Begin("Change parent", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
    //        ImGui::Text("#%i", m_UIDraggingEntity);
    //        ImGui::End();
    //    }
    //}

    ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_Once);
    std::string nodeTitle;
    const std::string& entityName = entity.World->GetName(entity);
    if (!entityName.empty()) {
        nodeTitle = entityName;
    } else {
        nodeTitle = std::string("#") + std::to_string(entity.ID);
    }
    if (ImGui::TreeNode(nodeTitle.c_str())) {
        //if (m_UIDraggingEntity != EntityID_Invalid && ImGui::IsItemHoveredRect() && ImGui::IsMouseReleased(0)) {
        //    LOG_DEBUG("Changed parent of %i to %i", m_UIDraggingEntity, entity);
        //    changeParent(m_UIDraggingEntity, entity);
        //    m_UIDraggingEntity = EntityID_Invalid;
        //}

        if (ImGui::BeginPopupContextItem("item context menu")) {
            if (ImGui::Button("Add")) {
                if (m_OnEntityCreate != nullptr) {
                    EntityWrapper newEntity = m_OnEntityCreate(EntityWrapper(entity.World, EntityID_Invalid));
                    ImGui::CloseCurrentPopup();
                    SelectEntity(newEntity);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                if (m_OnEntityDelete != nullptr) {
                    m_OnEntityDelete(entity);
                    ImGui::CloseCurrentPopup();
                } 
                if (!m_CurrentSelection.Valid()) {
                    SelectEntity(EntityWrapper::Invalid);
                }
            }
            ImGui::EndPopup();
        }
        return true;
    } else {
        return false;
    }
}

void EditorGUI::drawComponents(EntityWrapper entity)
{
    std::stringstream title;
    title << "Components";
    if (entity.Valid()) {
        title << " #" << entity.ID << "###Components";
    }
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
    // Draw combo box
    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 10.f);
    int selectedItem = -1;
    if (ImGui::Combo("", &selectedItem, componentTypes.data(), componentTypes.size())) {
        if (selectedItem != -1) {
            if (m_OnComponentAttach != nullptr) {
                std::string chosenComponentType(componentTypes.at(selectedItem));
                m_OnComponentAttach(entity, chosenComponentType);
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
        // TODO: Add delete button here
        drawComponent(entity, pool->ComponentInfo());
    }

    ImGui::End();
}

bool EditorGUI::drawComponent(EntityWrapper entity, const ComponentInfo& ci)
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
    for (auto& kv : ci.Fields) {
        const std::string& fieldName = kv.first;
        const ComponentInfo::Field_t& field = kv.second;

        // Draw the field widget based on its type
        drawComponentField(component, field);
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

    return true;
}

void EditorGUI::drawComponentField(ComponentWrapper& c, const ComponentInfo::Field_t& field)
{
    // Push an unique widget id so different components with fields with equal names are still counted as different
    ImGui::PushID((c.Info.Name + field.Name).c_str());

    if (field.Type == "Vector") {
        drawComponentField_Vector(c, field);
    } else if (field.Type == "Color") {
        drawComponentField_Color(c, field);
    //} else if (field.Type == "Quaternion") {
    } else if (field.Type == "int") {
        drawComponentField_int(c, field);
    } else if (field.Type == "enum") {
        drawComponentField_enum(c, field);
    } else if (field.Type == "float") {
        drawComponentField_float(c, field);
    } else if (field.Type == "double") {
        drawComponentField_double(c, field);
    } else if (field.Type == "bool") {
        drawComponentField_bool(c, field);
    } else if (field.Type == "string") {
        drawComponentField_string(c, field);
    } else {
        ImGui::TextDisabled(field.Type.c_str());
    }

    ImGui::PopID();
}

void EditorGUI::drawComponentField_Vector(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<glm::vec3>(field.Name);
    if (field.Name == "Scale") {
        // Limit scale values to a minimum of 0
        ImGui::DragFloat3("", glm::value_ptr(val), 0.1f, 0.f, std::numeric_limits<float>::max());
    } else if (field.Name == "Orientation") {
        // Make orentations have a period of 2*Pi
        glm::vec3 tempVal = glm::fmod(val, glm::vec3(glm::two_pi<float>()));
        if (ImGui::SliderFloat3("", glm::value_ptr(tempVal), 0.f, glm::two_pi<float>())) {
            val = tempVal;
        }
    } else {
        ImGui::DragFloat3("", glm::value_ptr(val), 0.1f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
    }
}

void EditorGUI::drawComponentField_Color(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<glm::vec4>(field.Name);
    ImGui::ColorEdit4("", glm::value_ptr(val), true);
}

void EditorGUI::drawComponentField_int(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<int>(field.Name);
    ImGui::InputInt("", &val);
}

void EditorGUI::drawComponentField_enum(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto fieldEnumDefIt = c.Info.Meta->FieldEnumDefinitions.find(field.Name);
    if (fieldEnumDefIt == c.Info.Meta->FieldEnumDefinitions.end()) {
        drawComponentField_int(c, field);
        return;
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
    }
    if (ImGui::Combo("", &selectedItem, enumKeys.str().c_str())) {
        val = enumValues.at(selectedItem);
    }
}

void EditorGUI::drawComponentField_float(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<float>(field.Name);
    ImGui::InputFloat("", &val, 0.01f, 1.f);
}

void EditorGUI::drawComponentField_double(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    float tempVal = static_cast<float>(c.Field<double>(field.Name));
    if (ImGui::InputFloat("", &tempVal, 0.01f, 1.f)) {
        c.SetField(field.Name, static_cast<double>(tempVal));
    }
}

void EditorGUI::drawComponentField_bool(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<bool>(field.Name);
    ImGui::Checkbox("", &val);
}

void EditorGUI::drawComponentField_string(ComponentWrapper &c, const ComponentInfo::Field_t &field)
{
    auto& val = c.Field<std::string>(field.Name);
    char tempString[1024]; // Let's just hope this is an sufficiently large buffer for strings :)
    tempString[1023] = '\0'; // Null terminator just in case the string is larger than the buffer
    // Copy the string into the buffer, taking the null terminator into account
    memcpy(tempString, val.c_str(), std::min(val.length() + 1, sizeof(tempString) - 1));
    if (ImGui::InputText("", tempString, sizeof(tempString))) {
        val = std::string(tempString);
    }
    // TODO: Handle drag and drop of files
}

