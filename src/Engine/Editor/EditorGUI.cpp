#include "Editor/EditorGUI.h"

void EditorGUI::Draw(World* world)
{
    drawMenu();
    drawEntities(world);
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
    ImRect bb(pos + ImVec2(20, 0), pos + ImVec2(width, 13));
    auto window = ImGui::GetCurrentWindow();
    if (m_CurrentSelection == entity) {
        const ImU32 col = window->Color(ImGuiCol_HeaderActive);
        window->DrawList->AddRectFilled(bb.Min, bb.Max, col);
    }
    ImGuiID id = window->GetID((std::string("#SelectButton") + std::to_string(entity)).c_str());
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
