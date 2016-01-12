#include "Editor/EditorSystem.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

EditorSystem::EditorSystem(EventBroker* eventBroker, IRenderer* renderer) 
    : ImpureSystem(eventBroker)
    , m_Renderer(renderer)
{
    auto config = ResourceManager::Load<ConfigFile>("Config.ini");
    m_Enabled = config->Get<bool>("Debug.EditorEnabled", false);
    m_Visible = m_Enabled;
    m_DefaultEntityDir = boost::filesystem::path("Schema") / boost::filesystem::path("Entities");

    if (!m_Enabled) {
        return;
    }

    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &EditorSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &EditorSystem::OnMousePress);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseRelease, &EditorSystem::OnMouseRelease);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseMove, &EditorSystem::OnMouseMove);
    EVENT_SUBSCRIBE_MEMBER(m_EPicking, &EditorSystem::OnPicking);
    EVENT_SUBSCRIBE_MEMBER(m_EFileDropped, &EditorSystem::OnFileDropped);
}

void EditorSystem::Update(World* world, double dt)
{
    m_World = world;

    if (!m_Enabled) {
        return;
    }

    if (!m_Visible) {
        return;
    }

    updateWidget();

    drawUI(world, dt);

    // Clear drop queue if it wasn't handled by any UI element
    if (!m_LastDroppedFile.empty()) {
        m_LastDroppedFile = "";
    }
}


boost::filesystem::path EditorSystem::openDialog(boost::filesystem::path defaultPath)
{
    namespace bfs = boost::filesystem;
    auto absolutePath = bfs::absolute(defaultPath);
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_OpenDialog(NULL, absolutePath.string().c_str(), &outPath);
    if (result == NFD_ERROR) {
        LOG_ERROR("NFD Error: %s", NFD_GetError());
        return bfs::path();
    }

    return bfs::absolute(outPath);
}

boost::filesystem::path EditorSystem::saveDialog(boost::filesystem::path defaultPath)
{
    namespace bfs = boost::filesystem;
    auto absolutePath = bfs::absolute(defaultPath);
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_SaveDialog(NULL, absolutePath.string().c_str(), &outPath);
    if (result == NFD_ERROR) {
        LOG_ERROR("NFD Error: %s", NFD_GetError());
        return bfs::path();
    }

    return bfs::absolute(outPath);
}

bool EditorSystem::OnInputCommand(const Events::InputCommand& e)
{
    if (e.Command == "ToggleEditor" && e.Value > 0) {
        m_Visible = !m_Visible;
    }

    if (e.Command == "EditorToolMove" && e.Value > 0) {
        setWidgetMode(WidgetMode::Translate);
    }
    if (e.Command == "EditorToolRotate" && e.Value > 0) {
        setWidgetMode(WidgetMode::Rotate);
    }
    if (e.Command == "EditorToolScale" && e.Value > 0) {
        setWidgetMode(WidgetMode::Scale);
    }

    if (e.Command == "EditorToggleTransformSpace" && e.Value > 0) {
        if (m_WidgetSpace == WidgetSpace::Global) {
            setWidgetSpace(WidgetSpace::Local);
        } else if (m_WidgetSpace == WidgetSpace::Local) {
            setWidgetSpace(WidgetSpace::Global);
        }
    }

    return true;
}

bool EditorSystem::OnMousePress(const Events::MousePress& e)
{
    if (e.Button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_PickingQueue.push_back(glm::vec2((int)e.X, (int)e.Y));
    }
    return true;
}

bool EditorSystem::OnMouseMove(const Events::MouseMove& e)
{
    if (m_Widget == EntityID_Invalid) {
        return false;
    }
    if (m_Selection == EntityID_Invalid) {
        return false;
    }
    if (m_Selection == m_Widget) {
        return false;
    }
    // TODO: No widgets for root entity until widgets reside in thier own world,
    // or the widgets will move relative to the root entity being moved, which is WEEEIRD.
    if (m_Selection == 0) {
        return false;
    }

    auto widgetTransform = m_World->GetComponent(m_Widget, "Transform");
    glm::vec3 widgetOrientation = widgetTransform["Orientation"];
    glm::quat totalOrientation = m_Renderer->Camera()->Orientation() * glm::inverse(glm::quat(widgetOrientation));

    int width;
    int height;
    glfwGetFramebufferSize(m_Renderer->Window(), &width, &height);
    Rectangle res(width, height);

    glm::vec2 delta2(res.Width / 2.f + e.DeltaX, res.Height / 2.f + -e.DeltaY);
    glm::vec3 deltaWorld = ScreenCoords::ToWorldPos(
        delta2,
        m_WidgetPickingDepth,
        res,
        m_Renderer->Camera()->ProjectionMatrix(),
        glm::toMat4(glm::inverse(totalOrientation))
    );
    glm::vec3 origin = ScreenCoords::ToWorldPos(
        glm::vec2(res.Width / 2.f, res.Height / 2.f),
        m_WidgetPickingDepth,
        res,
        m_Renderer->Camera()->ProjectionMatrix(),
        glm::toMat4(glm::inverse(totalOrientation))
    );
    deltaWorld = deltaWorld - origin;
    glm::vec3 movement = deltaWorld * m_WidgetCurrentAxis;

    if (glm::length2(m_WidgetCurrentAxis) > 0.f) {
        auto widgetTransform = m_World->GetComponent(m_Widget, "Transform");
        if (m_WidgetMode == WidgetMode::Translate) {
            if (m_WidgetSpace == WidgetSpace::Global) {
                EntityID parent = m_World->GetParent(m_Selection);
                glm::quat inverseParentOrientation;
                //if (parent != 0) {
                    inverseParentOrientation = glm::inverse(RenderQueueFactory::AbsoluteOrientation(m_World, parent));
                //}
                (glm::vec3&)m_World->GetComponent(m_Selection, "Transform")["Position"] += inverseParentOrientation * movement;
            } else if (m_WidgetSpace == WidgetSpace::Local) {
                auto selectionTransform = m_World->GetComponent(m_Selection, "Transform");
                (glm::vec3&)selectionTransform["Position"] += glm::quat((glm::vec3)selectionTransform["Orientation"]) * movement;
            }
        } else if (m_WidgetMode == WidgetMode::Rotate) {
            glm::vec3 finalMovement;
            finalMovement.x = -deltaWorld.y * m_WidgetCurrentAxis.x;
            finalMovement.y = deltaWorld.x * m_WidgetCurrentAxis.y;
            finalMovement.z = deltaWorld.y * m_WidgetCurrentAxis.z;
            if (m_WidgetSpace == WidgetSpace::Global) {
                EntityID parent = m_World->GetParent(m_Selection);
                glm::quat parentOrientation;
                //if (parent != 0) {
                //    parentOrientation = RenderQueueFactory::AbsoluteOrientation(m_World, parent);
                //}
                glm::vec3& selectionOrientation = m_World->GetComponent(m_Selection, "Transform")["Orientation"];
                glm::quat currentOrientation = RenderQueueFactory::AbsoluteOrientation(m_World, m_Selection);
                //glm::quat currentOrientation = parentOrientation * glm::quat(selectionOrientation);
                glm::quat deltaOrientation(finalMovement);
                selectionOrientation = glm::eulerAngles(glm::inverse(parentOrientation) * (deltaOrientation * currentOrientation));
            } else if (m_WidgetSpace == WidgetSpace::Local) {
                glm::vec3& selectionOrientation = m_World->GetComponent(m_Selection, "Transform")["Orientation"];
                glm::quat currentOrientation(selectionOrientation);
                glm::quat deltaOrientation(finalMovement);
                selectionOrientation = glm::eulerAngles(currentOrientation * deltaOrientation);
            }
        } else if (m_WidgetMode == WidgetMode::Scale) {
            glm::vec3& scaleX = m_World->GetComponent(m_WidgetX, "Transform")["Scale"];
            glm::vec3& scaleY = m_World->GetComponent(m_WidgetY, "Transform")["Scale"];
            glm::vec3& scaleZ = m_World->GetComponent(m_WidgetZ, "Transform")["Scale"];

            if (m_WidgetCurrentAxis.x > 0 && m_WidgetCurrentAxis.y > 0 && m_WidgetCurrentAxis.z > 0) {
                float movementLength = glm::length(movement);
                float dot = glm::dot((glm::vec3)widgetOrientation, movement);
                movement = glm::vec3(movementLength) * glm::sign(dot);
                (glm::vec3&)m_World->GetComponent(m_WidgetOrigin, "Transform")["Scale"] += movement;
            }
            if (m_WidgetCurrentAxis.x > 0) {
                scaleX.x += movement.x;
            }
            if (m_WidgetCurrentAxis.y > 0) {
                scaleY.y += movement.y;
            }
            if (m_WidgetCurrentAxis.z > 0) {
                scaleZ.z += movement.z;
            }
            (glm::vec3&)m_World->GetComponent(m_Selection, "Transform")["Scale"] += movement;
        }
    }


    /*LOG_DEBUG("DELTA %f", e.DeltaX);
    if (e.X < 0) {
        glfwSetCursorPos(m_Renderer->Window(), width - 1, e.Y);
    }
    if (e.X >= width) {
        glfwSetCursorPos(m_Renderer->Window(), 0, e.Y);
    }*/

    return true;
}

bool EditorSystem::OnMouseRelease(const Events::MouseRelease& e)
{
    if (glm::length2(m_WidgetCurrentAxis) > 0.f) {
        m_WidgetCurrentAxis = glm::vec3(0.f);
        //setWidgetMode(m_WidgetMode);
    }

    return true;
}

bool EditorSystem::OnPicking(const Events::Picking& e)
{
    for (auto& pos : m_PickingQueue) {
        auto result = e.Pick(pos);
        EntityID entity = result.Entity;
        if (glm::length2(m_WidgetCurrentAxis) > 0.f) {
            // ???
        } else {
            LOG_INFO("Selected %i", entity);
            if (entity != EntityID_Invalid) {
                EntityID parent = m_World->GetParent(entity);
                if (parent == m_Widget) {
                    m_WidgetCurrentAxis = glm::vec3(
                        (entity == m_WidgetX) || (entity == m_WidgetOrigin) || (entity == m_WidgetPlaneY || entity == m_WidgetPlaneZ),
                        (entity == m_WidgetY) || (entity == m_WidgetOrigin) || (entity == m_WidgetPlaneX || entity == m_WidgetPlaneZ),
                        (entity == m_WidgetZ) || (entity == m_WidgetOrigin) || (entity == m_WidgetPlaneX || entity == m_WidgetPlaneY)
                    );
                    m_WidgetPickingDepth = result.Depth;

                    //auto widgetTransform = m_World->GetComponent(m_Widget, "Transform");
                    //auto selectionTransform = m_World->GetComponent(m_Selection, "Transform");
                    //widgetTransform["Position"] = (glm::vec3)selectionTransform["Position"];
                } else {
                    ImGui::SetActiveID(0, nullptr);
                    if (m_WidgetMode == WidgetMode::None) {
                        m_WidgetMode = WidgetMode::Translate;
                    }
                    setWidgetMode(m_WidgetMode);
                    m_Selection = entity;
                }
            }
        }
    }
    m_PickingQueue.clear();
    return true;
};

bool EditorSystem::OnFileDropped(const Events::FileDropped& e)
{
    m_LastDroppedFile = boost::filesystem::path(e.Path).lexically_relative(boost::filesystem::current_path()).string();
    std::replace(m_LastDroppedFile.begin(), m_LastDroppedFile.end(), '\\', '/');
    return true;
}

void EditorSystem::createWidget()
{
    if (m_Widget == EntityID_Invalid) {
        m_Widget = m_World->CreateEntity();
        m_World->AttachComponent(m_Widget, "Transform");
        m_WidgetX = m_World->CreateEntity(m_Widget);
        m_World->AttachComponent(m_WidgetX, "Transform");
        m_World->AttachComponent(m_WidgetX, "Model");
        m_WidgetPlaneX = m_World->CreateEntity(m_Widget);
        m_World->AttachComponent(m_WidgetPlaneX, "Transform");
        m_World->AttachComponent(m_WidgetPlaneX, "Model");
        m_World->GetComponent(m_WidgetPlaneX, "Model")["Resource"] = "Models/WidgetPlaneX.obj";
        m_WidgetY = m_World->CreateEntity(m_Widget);
        m_World->AttachComponent(m_WidgetY, "Transform");
        m_World->AttachComponent(m_WidgetY, "Model");
        m_WidgetPlaneY = m_World->CreateEntity(m_Widget);
        m_World->AttachComponent(m_WidgetPlaneY, "Transform");
        m_World->AttachComponent(m_WidgetPlaneY, "Model");
        m_World->GetComponent(m_WidgetPlaneY, "Model")["Resource"] = "Models/WidgetPlaneY.obj";
        m_WidgetZ = m_World->CreateEntity(m_Widget);
        m_World->AttachComponent(m_WidgetZ, "Transform");
        m_World->AttachComponent(m_WidgetZ, "Model");
        m_WidgetPlaneZ = m_World->CreateEntity(m_Widget);
        m_World->AttachComponent(m_WidgetPlaneZ, "Transform");
        m_World->AttachComponent(m_WidgetPlaneZ, "Model");
        m_World->GetComponent(m_WidgetPlaneZ, "Model")["Resource"] = "Models/WidgetPlaneZ.obj";
        m_WidgetOrigin = m_World->CreateEntity(m_Widget);
        m_World->AttachComponent(m_WidgetOrigin, "Transform");
        m_World->AttachComponent(m_WidgetOrigin, "Model");
        setWidgetMode(WidgetMode::None);
    }
}

void EditorSystem::updateWidget()
{
    if (m_Widget == EntityID_Invalid) {
        return;
    }
    if (m_Selection == m_Widget) {
        return;
    }

    if (m_Selection != EntityID_Invalid) {
        auto widgetTransform = m_World->GetComponent(m_Widget, "Transform");
        glm::vec3 selectionPosition = RenderQueueFactory::AbsolutePosition(m_World, m_Selection);
        widgetTransform["Position"] = selectionPosition;
        if (m_WidgetSpace == WidgetSpace::Local) {
            widgetTransform["Orientation"] = glm::eulerAngles(RenderQueueFactory::AbsoluteOrientation(m_World, m_Selection));
        }
    }
}

void EditorSystem::setWidgetMode(WidgetMode newMode)
{
    if (m_Widget == EntityID_Invalid) {
        return;
    }

    auto widgetTransform = m_World->GetComponent(m_Widget, "Transform");
    widgetTransform["Orientation"] = glm::vec3(0.f);
    m_World->GetComponent(m_WidgetX, "Transform")["Scale"] = glm::vec3(1.f);
    m_World->GetComponent(m_WidgetPlaneX, "Model")["Visible"] = false;
    m_World->GetComponent(m_WidgetY, "Transform")["Scale"] = glm::vec3(1.f);
    m_World->GetComponent(m_WidgetPlaneY, "Model")["Visible"] = false;
    m_World->GetComponent(m_WidgetZ, "Transform")["Scale"] = glm::vec3(1.f);
    m_World->GetComponent(m_WidgetPlaneZ, "Model")["Visible"] = false;
    m_World->GetComponent(m_WidgetOrigin, "Transform")["Scale"] = glm::vec3(1.f);
    m_World->GetComponent(m_WidgetOrigin, "Model")["Visible"] = false;

    if (newMode == WidgetMode::Translate) {
        m_World->GetComponent(m_WidgetX, "Model")["Resource"] = "Models/TranslationWidgetX.obj";
        m_World->GetComponent(m_WidgetY, "Model")["Resource"] = "Models/TranslationWidgetY.obj";
        m_World->GetComponent(m_WidgetZ, "Model")["Resource"] = "Models/TranslationWidgetZ.obj";
        // Temporarily disabled for local space until I can figure out what's wrong with the math 
        if (m_WidgetSpace != WidgetSpace::Local) {
            m_World->GetComponent(m_WidgetPlaneX, "Model")["Visible"] = true;
            m_World->GetComponent(m_WidgetPlaneY, "Model")["Visible"] = true;
            m_World->GetComponent(m_WidgetPlaneZ, "Model")["Visible"] = true;
        }
        if (m_Selection != EntityID_Invalid) {
            if (m_WidgetSpace == WidgetSpace::Local) {
                auto selectionTransform = m_World->GetComponent(m_Selection, "Transform");
                widgetTransform["Orientation"] = glm::eulerAngles(RenderQueueFactory::AbsoluteOrientation(m_World, m_Selection));
            }
        }
    } else if (newMode == WidgetMode::Scale) {
        m_World->GetComponent(m_WidgetX, "Model")["Resource"] = "Models/ScaleWidgetX.obj";
        m_World->GetComponent(m_WidgetY, "Model")["Resource"] = "Models/ScaleWidgetY.obj";
        m_World->GetComponent(m_WidgetZ, "Model")["Resource"] = "Models/ScaleWidgetZ.obj";
        m_World->GetComponent(m_WidgetOrigin, "Model")["Visible"] = true;
        m_World->GetComponent(m_WidgetOrigin, "Model")["Resource"] = "Models/ScaleWidgetOrigin.obj";
        if (m_Selection != EntityID_Invalid) {
            auto selectionTransform = m_World->GetComponent(m_Selection, "Transform");
            widgetTransform["Orientation"] = glm::eulerAngles(RenderQueueFactory::AbsoluteOrientation(m_World, m_Selection));
        }
    } else if (newMode == WidgetMode::Rotate) {
        m_World->GetComponent(m_WidgetX, "Model")["Resource"] = "Models/RotationWidgetX.obj";
        m_World->GetComponent(m_WidgetY, "Model")["Resource"] = "Models/RotationWidgetY.obj";
        m_World->GetComponent(m_WidgetZ, "Model")["Resource"] = "Models/RotationWidgetZ.obj";
        if (m_Selection != EntityID_Invalid) {
            auto selectionTransform = m_World->GetComponent(m_Selection, "Transform");
            if (m_WidgetSpace == WidgetSpace::Local) {
                widgetTransform["Orientation"] = glm::eulerAngles(RenderQueueFactory::AbsoluteOrientation(m_World, m_Selection));
            }
        }
    }
    m_WidgetMode = newMode;
}

void EditorSystem::setWidgetSpace(WidgetSpace space)
{
    m_WidgetSpace = space;
    setWidgetMode(m_WidgetMode);
}

void EditorSystem::drawUI(World* world, double dt)
{
    namespace bfs = boost::filesystem;

    ImGui::ShowTestWindow();
    //ImGui::ShowStyleEditor();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            //if (ImGui::MenuItem("New")) { }
            if (ImGui::MenuItem("Import", "Ctrl+O")) {
                fileImport(world);
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                fileSave(world);
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                fileSaveAs(world);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close Editor", "F1")) { }

            ImGui::EndMenu();
        }

        ImGui::SameLine();
        if (ImGui::Button("Move")) {
            setWidgetMode(WidgetMode::Translate);
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate")) {
            setWidgetMode(WidgetMode::Rotate);
        }
        ImGui::SameLine();
        if (ImGui::Button("Scale")) {
            setWidgetMode(WidgetMode::Scale);
        }
        ImGui::SameLine();
        if (m_WidgetSpace == WidgetSpace::Global) {
            if (ImGui::Button("(Global)")) {
                setWidgetSpace(WidgetSpace::Local);
            }
        } else if (m_WidgetSpace == WidgetSpace::Local) {
            if (ImGui::Button("(Local)")) {
                setWidgetSpace(WidgetSpace::Global);
            }
        }

        ImGui::EndMainMenuBar();
    }

    std::string title = std::string("Components #") + std::to_string(m_Selection) + std::string("###Components");
    if (ImGui::Begin(title.c_str())) {
        if (m_Selection != EntityID_Invalid) {
            auto& pools = world->GetComponentPools();

            std::vector<const char*> componentTypes;
            for (auto& pair : pools) {
                // Only add components the entity doesn't already have
                if (!pair.second->KnowsEntity(m_Selection)) {
                    componentTypes.push_back(pair.first.c_str());
                }
            }
            int item = -1;
            ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() - 5.f);
            if (ImGui::Combo("", &item, componentTypes.data(), componentTypes.size())) {
                if (item != -1) {
                    std::string chosenType = std::string(componentTypes.at(item));
                    world->AttachComponent(m_Selection, chosenType);
                }
            }
            ImGui::PopItemWidth();

            for (auto& pair : pools) {
                const std::string& componentType = pair.first;
                auto pool = pair.second;
                if (!pool->KnowsEntity(m_Selection)) {
                    continue;
                }
                auto& ci = pool->ComponentInfo();

                bool deletePressed = createDeleteButton(componentType);
                if (deletePressed) {
                    world->DeleteComponent(m_Selection, componentType);
                    continue;
                }

                if (ImGui::CollapsingHeader(componentType.c_str())) {
                    if (!ci.Meta.Annotation.empty()) {
                        ImGui::Text(ci.Meta.Annotation.c_str());
                    }

                    auto& component = world->GetComponent(m_Selection, componentType);
                    for (auto& kv : ci.Fields) {
                        const std::string& fieldName = kv.first;
                        auto& field = kv.second;
                        
                        ImGui::PushID(fieldName.c_str());
                        if (field.Type == "Vector") {
                            auto& val = component.Property<glm::vec3>(fieldName);
                            if (fieldName == "Scale") {
                                ImGui::DragFloat3("", glm::value_ptr(val), 0.1f, 0.f, std::numeric_limits<float>::max());
                            } else if (fieldName == "Orientation") {
                                glm::vec3 tempVal = glm::fmod(val, glm::vec3(glm::two_pi<float>()));
                                if (ImGui::SliderFloat3("", glm::value_ptr(tempVal), 0.f, glm::two_pi<float>())) {
                                    val = tempVal;
                                }
                            } else {
                                ImGui::DragFloat3("", glm::value_ptr(val), 0.1f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
                            }
                        } else if (field.Type == "Color") {
                            auto& val = component.Property<glm::vec4>(fieldName);
                            ImGui::ColorEdit4("", glm::value_ptr(val), true);
                        } else if (field.Type == "string") {
                            std::string& val = component.Property<std::string>(fieldName);
                            char tempString[1024];
                            memcpy(tempString, val.c_str(), std::min(val.length() + 1, sizeof(tempString)));
                            if (ImGui::InputText("", tempString, sizeof(tempString))) {
                                val = std::string(tempString);
                                LOG_DEBUG("%s::%s changed!", componentType.c_str(), fieldName.c_str());
                            }
                            // DROP STUFF
                            if (ImGui::IsItemHovered() && !m_LastDroppedFile.empty()) {
                                val = m_LastDroppedFile;
                                m_LastDroppedFile = "";
                            }

                        } else if (field.Type == "double") {
                            float tempVal = static_cast<float>(component.Property<double>(fieldName));
                            if (ImGui::InputFloat("", &tempVal, 0.01f, 1.f)) {
                                component.SetProperty(fieldName, static_cast<double>(tempVal));
                            }
                        } else if (field.Type == "bool") {
                            auto& val = component.Property<bool>(fieldName);
                            ImGui::Checkbox("", &val);
                        } else {
                            ImGui::TextDisabled(field.Type.c_str());
                        }
                        ImGui::PopID();

                        ImGui::SameLine();
                        ImGui::Text(fieldName.c_str());
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("field annotation goes here");
                        }
                    }
                }
            }
        }

    }
    ImGui::End();

    if (ImGui::Begin("Entities")) {
        auto entityChildren = world->GetEntityChildren();
        std::function<void(EntityID)> recurse = [&](EntityID parent) {
            auto range = entityChildren.equal_range(parent);
            for (auto it = range.first; it != range.second; it++) {
                if (createEntityNode(world, it->second)) {
                    recurse(it->second);
                    ImGui::TreePop();
                }
            }
        };
        recurse(EntityID_Invalid);
    }
    ImGui::End();
}

bool EditorSystem::createEntityNode(World* world, EntityID entity)
{
    // HACK: Don't show the widget entities in the entity tree
    if (entity == m_Widget) {
        return false;
    }

    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvailWidth();
    ImRect bb(pos + ImVec2(20, 0), pos + ImVec2(width, 13));
    auto window = ImGui::GetCurrentWindow();
    if (m_Selection == entity) {
        const ImU32 col = window->Color(ImGuiCol_HeaderActive);
        window->DrawList->AddRectFilled(bb.Min, bb.Max, col);
    }
    ImGuiID id = window->GetID((std::string("#SelectButton") + std::to_string(entity)).c_str());
    bool hovered = false;
    bool held = false;
    if (ImGui::ButtonBehavior(bb, id, &hovered, &held)) {
        m_Selection = entity;
    }
    if (held) {
        ImVec2 entityDragDelta = ImGui::GetMouseDragDelta(0);
        if (std::abs(entityDragDelta.x) > 0 && std::abs(entityDragDelta.y) > 0) {
            if (m_UIDraggingEntity == EntityID_Invalid) {
                m_UIDraggingEntity = entity;
                LOG_DEBUG("Started drag of entity %i", m_UIDraggingEntity);
            }
            ImGui::SetNextWindowPos(ImGui::GetIO().MousePos + ImVec2(20, 0));
            ImGui::Begin("Change parent", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
            ImGui::Text("#%i", m_UIDraggingEntity);
            ImGui::End();
        }
    }

    ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_Once);
    std::string nodeTitle;
    const std::string& entityName = world->GetName(entity);
    if (!entityName.empty()) {
        nodeTitle = entityName;
    } else {
        nodeTitle = std::string("#") + std::to_string(entity);
    }
    if (ImGui::TreeNode(nodeTitle.c_str())) {
        if (m_UIDraggingEntity != EntityID_Invalid && ImGui::IsItemHoveredRect() && ImGui::IsMouseReleased(0)) {
            LOG_DEBUG("Changed parent of %i to %i", m_UIDraggingEntity, entity);
            changeParent(m_UIDraggingEntity, entity);
            m_UIDraggingEntity = EntityID_Invalid;
        }

        if (ImGui::BeginPopupContextItem("item context menu")) {
            if (ImGui::Button("Add")) {
                EntityID newEntity = world->CreateEntity(entity);
                world->AttachComponent(newEntity, "Transform");
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                world->DeleteEntity(entity);
                ImGui::CloseCurrentPopup();
                if (!world->ValidEntity(m_Selection)) {
                    m_Selection = EntityID_Invalid;
                }
            }
            ImGui::EndPopup();
        }
        return true;
    } else {
        return false;
    }
}

bool EditorSystem::createDeleteButton(std::string componentType)
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

void EditorSystem::changeParent(EntityID entity, EntityID newParent)
{
    if (entity == newParent) {
        return;
    }

    // An entity can't be a child to one of its own children
    auto children = m_World->GetEntityChildren().equal_range(entity);
    for (auto it = children.first; it != children.second; it++) {
        if (it->second == newParent) {
            return;
        }
    }

    m_World->SetParent(entity, newParent);
}

void EditorSystem::fileImport(World* world)
{
    m_CurrentFile = openDialog(m_DefaultEntityDir);
    auto file = ResourceManager::Load<EntityFile>(m_CurrentFile.string());
    EntityFilePreprocessor fpp(file);
    fpp.RegisterComponents(world);
    EntityFileParser fp(file);
    fp.MergeEntities(world);
    createWidget();
    updateWidget();
}

void EditorSystem::fileSave(World* world)
{
    if (boost::filesystem::exists(m_CurrentFile)) {
        // HACK: Delete the widgets so they don't appear in the saved file
        world->DeleteEntity(m_Widget);
        m_Widget = EntityID_Invalid;

        EntityFileWriter writer(m_CurrentFile.string());
        writer.WriteWorld(world);

        createWidget();
    } else {
        fileSaveAs(world);
    }
}

void EditorSystem::fileSaveAs(World* world)
{
    auto filePath = saveDialog(m_DefaultEntityDir);
    if (filePath.empty()) {
        return;
    }

    // HACK: Delete the widgets so they don't appear in the saved file
    world->DeleteEntity(m_Widget);
    m_Widget = EntityID_Invalid;

    EntityFileWriter writer(filePath.string());
    writer.WriteWorld(world);

    createWidget();
}
