#include "Core/EntityWrapper.h"
#include "Core/World.h"

const EntityWrapper EntityWrapper::Invalid = EntityWrapper(nullptr, EntityID_Invalid);

bool EntityWrapper::HasComponent(const std::string& componentName)
{
    if (!Valid()) {
        return false;
    }
    return World->HasComponent(ID, componentName);
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
    auto itPair = this->World->GetChildren(this->ID);
    if (itPair.first == itPair.second) {
        return EntityWrapper::Invalid;
    }

    for (auto it = itPair.first; it != itPair.second; ++it) {
        if (this->World->GetName(it->second) == name) {
            return EntityWrapper(this->World, it->second);
        }
    }

    return EntityWrapper::Invalid;
}

bool EntityWrapper::Valid()
{
    if (this->World == nullptr) {
        return false;
    }

    if (this->ID == EntityID_Invalid) {
        return false;
    }

    if (!this->World->ValidEntity(this->ID)) {
        this->ID = EntityID_Invalid;
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

EntityWrapper::operator bool()
{
    return this->Valid();
}

