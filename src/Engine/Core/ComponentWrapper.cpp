#include "Core/World.h"
#include "Core/ComponentWrapper.h"

ComponentWrapper::ComponentWrapper(const ComponentInfo& componentInfo, char* data, ::DirtyBitField* dirtyBitField, World* world)
    : Info(componentInfo)
    , EntityID(*reinterpret_cast<::EntityID*>(data))
    , Data(data + componentInfo.GetHeaderSize())
    , DirtyBitField(dirtyBitField)
    , m_World(world)
{}

ComponentInfo::EnumType ComponentWrapper::Enum(const char* fieldName, const char* enumKey)
{
    return Info.Meta->FieldEnumDefinitions.at(fieldName).at(enumKey);
}

bool ComponentWrapper::Dirty(DirtySetType type, const std::string& fieldName)
{
    if (DirtyBitField == nullptr) {
        return true;
    } else {
        auto& field = Info.Fields.at(fieldName);
        return DirtyBitField->operator[](type).count(field.Index) == 1;
    }
}

void ComponentWrapper::SetDirty(DirtySetType type, const std::string& fieldName, bool dirty /* = true */)
{
    if (DirtyBitField == nullptr) {
        return;
    }

    auto& field = Info.Fields.at(fieldName);
    if (dirty) {
        DirtyBitField->operator[](type).insert(field.Index);
        // Because parents affects children when altered, we also need to flag all children as dirty.
        if (type == DirtySetType::Transform && m_World != nullptr) {
            auto children = m_World->GetDirectChildren(EntityID);
            for (auto kv = children.first; kv != children.second; ++kv) {
                const auto& child = kv->second;
                if (m_World->HasComponent(child, Info.Name)) {
                    m_World->GetComponent(child, Info.Name).SetDirty(DirtySetType::Transform, fieldName, true);
                    if (fieldName == "Orientation" || fieldName == "Scale") {
                        m_World->GetComponent(child, Info.Name).SetDirty(DirtySetType::Transform, "Position", true);
                    }
                }
            }
        }
    } else {
        DirtyBitField->operator[](type).erase(field.Index);
    }
}

void ComponentWrapper::SetAllDirty(const std::string& fieldName, bool dirty /* = true */)
{
    for (auto& kv : *DirtyBitField) {
        SetDirty(kv.first, fieldName);
    }
}