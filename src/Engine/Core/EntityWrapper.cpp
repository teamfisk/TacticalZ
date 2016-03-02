#include "Core/EntityWrapper.h"
#include "Core/World.h"

const EntityWrapper EntityWrapper::Invalid = EntityWrapper(nullptr, EntityID_Invalid);

const std::string EntityWrapper::Name()
{
    return World->GetName(ID);
}

bool EntityWrapper::HasComponent(const std::string& componentName)
{
    if (!Valid()) {
        return false;
    }
    return World->HasComponent(ID, componentName);
}

void EntityWrapper::AttachComponent(const char* componentName)
{
    if (!Valid()) {
        LOG_WARNING("Could not attach \"%s\" component to #%i, entity is not valid.", componentName, ID);
        return;
    }
    World->AttachComponent(ID, componentName);
}

EntityWrapper EntityWrapper::Parent()
{
    if (this->World == nullptr || this->ID == EntityID_Invalid) {
        return EntityWrapper::Invalid;
    } else {
        return EntityWrapper(this->World, this->World->GetParent(this->ID));
    }
}

EntityWrapper EntityWrapper::FirstChildByName(const std::string& name)
{
    return firstChildByNameRecursive(name, this->ID);
}


EntityWrapper EntityWrapper::FirstLevelChildByName(const std::string& name)
{
    EntityID parent = this->ID;
    if (!this->World->ValidEntity(parent)) {
        return EntityWrapper::Invalid;
    }

    auto itPair = this->World->GetDirectChildren(parent);
    if (itPair.first == itPair.second) {
        return EntityWrapper::Invalid;
    }

    for (auto it = itPair.first; it != itPair.second; ++it) {
        std::string itName = this->World->GetName(it->second);
        if (itName == name) {
            return EntityWrapper(this->World, it->second);
        } else if (it->second != EntityID_Invalid) {
            continue;
        }
    }

    return EntityWrapper::Invalid;
}

EntityWrapper EntityWrapper::FirstParentWithComponent(const std::string& componentType)
{
    EntityWrapper entity = *this;
    while (entity.Parent().Valid()) {
        entity = entity.Parent();
        if (entity.HasComponent(componentType)) {
            return entity;
        }
    }
    return EntityWrapper::Invalid;
}

EntityWrapper EntityWrapper::Clone(EntityWrapper parent /*= Invalid*/)
{
    if (!Valid()) {
        return EntityWrapper::Invalid;
    }

    EntityWrapper clone = cloneRecursive(*this, EntityWrapper::Invalid);
    this->World->SetParent(clone.ID, parent.ID);
    return clone;
}

std::vector<EntityWrapper> EntityWrapper::ChildrenWithComponent(const std::string& componentType)
{
    std::vector<EntityWrapper> childrenWithComponent;
    childrenWithComponentRecursive(componentType, *this, childrenWithComponent);
    return childrenWithComponent;
}

void EntityWrapper::DeleteChildren()
{
    auto itPair = this->World->GetDirectChildren(this->ID);
    if (itPair.first == itPair.second) {
        return;
    }

    std::vector<EntityID> entitiesToDelete;
    for (auto it = itPair.first; it != itPair.second; it++) {
        entitiesToDelete.push_back(it->second);
    }
    for (auto& e : entitiesToDelete) {
        this->World->DeleteEntity(e);
    }
}

bool EntityWrapper::IsChildOf(EntityWrapper potentialParent)
{
    EntityWrapper entity = *this;
    while (entity.Parent().Valid()) {
        entity = entity.Parent();
        if (entity == potentialParent) {
            return true;
        }
    }
    return false;
}

bool EntityWrapper::Valid() const
{
    if (this->World == nullptr) {
        return false;
    }

    if (this->ID == EntityID_Invalid) {
        return false;
    }

    if (!this->World->ValidEntity(this->ID)) {
        return false;
    }

    return true;
}

ComponentWrapper EntityWrapper::operator[](const char* componentName)
{
    if (World->HasComponent(ID, componentName)) {
        return World->GetComponent(ID, componentName);
    } else {
        LOG_WARNING("EntityWrapper implicitly attached \"%s\" component to #%i as a result of a fetch request!", componentName, ID);
        return World->AttachComponent(ID, componentName);
    }
}

ComponentWrapper EntityWrapper::operator[](const std::string& componentName)
{
    return this->operator[](componentName.c_str());
}

bool EntityWrapper::operator==(const EntityWrapper& e) const
{
    return (this->ID == e.ID) && (this->World == e.World);
}

bool EntityWrapper::operator!=(const EntityWrapper& e) const
{
    return !this->operator==(e);
}

EntityWrapper::operator EntityID() const
{
    return this->ID;
}

EntityWrapper EntityWrapper::firstChildByNameRecursive(const std::string& name, EntityID parent)
{
    if (!this->World->ValidEntity(parent)) {
        return EntityWrapper::Invalid;
    }

    auto itPair = this->World->GetDirectChildren(parent);
    if (itPair.first == itPair.second) {
        return EntityWrapper::Invalid;
    }

    for (auto it = itPair.first; it != itPair.second; ++it) {
        std::string itName = this->World->GetName(it->second);
        if (itName == name) {
            return EntityWrapper(this->World, it->second);
        } else if (it->second != EntityID_Invalid) {
            EntityWrapper result = firstChildByNameRecursive(name, it->second);
            if (result != EntityWrapper::Invalid) {
                return result;
            }
        }
    }

    return EntityWrapper::Invalid;
}

EntityWrapper EntityWrapper::cloneRecursive(EntityWrapper entity, EntityWrapper parent)
{
    EntityWrapper clone = EntityWrapper(entity.World, entity.World->CreateEntity(parent.ID));
    entity.World->SetName(clone.ID, entity.Name());

    // Clone components
    for (auto& kv : entity.World->GetComponentPools()) {
        if (kv.second->KnowsEntity(entity.ID)) {
            ComponentWrapper c1 = kv.second->GetByEntity(entity.ID);
            ComponentWrapper c2 = entity.World->AttachComponent(clone.ID, kv.first);
            c1.Copy(c2);
        }
    }

    // Clone children
    auto children = entity.World->GetDirectChildren(entity.ID);
    for (auto it = children.first; it != children.second; ++it) {
        EntityWrapper child(entity.World, it->second);
        cloneRecursive(child, clone);
    }
      
    return clone;
}

void EntityWrapper::childrenWithComponentRecursive(const std::string& componentType, EntityWrapper& entity, std::vector<EntityWrapper>& childrenWithComponent)
{
    auto itPair = this->World->GetDirectChildren(entity.ID);
    if (itPair.first == itPair.second) {
        return;
    }

    for (auto it = itPair.first; it != itPair.second; ++it) {
        EntityWrapper child = EntityWrapper(entity.World, it->second);
        if (child.HasComponent(componentType)) {
            childrenWithComponent.push_back(child);
        }
        childrenWithComponentRecursive(componentType, child, childrenWithComponent);
    }
}
