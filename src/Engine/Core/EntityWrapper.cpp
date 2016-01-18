#include "Core/EntityWrapper.h"
#include "Core/World.h"

const EntityWrapper EntityWrapper::Invalid = EntityWrapper(nullptr, EntityID_Invalid);

bool EntityWrapper::HasComponent(const std::string& componentName)
{
    return World->HasComponent(ID, componentName);
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
    return (this->World == e.World) && (this->ID == e.ID);
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

