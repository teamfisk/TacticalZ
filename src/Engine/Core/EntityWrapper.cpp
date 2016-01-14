#include "Core/EntityWrapper.h"
#include "Core/World.h"

bool EntityWrapper::operator==(const EntityWrapper& e)
{
    return (this->World == e.World) && (this->ID == e.ID);
}

bool EntityWrapper::HasComponent(const std::string& componentName)
{
    return World->HasComponent(ID, componentName);
}

ComponentWrapper EntityWrapper::operator[](const std::string& componentName)
{
    if (World->HasComponent(ID, componentName)) {
        return World->GetComponent(ID, componentName);
    } else {
        LOG_WARNING("EntityWrapper implicitly attached \"%s\" component to #%i as a result of a fetch request!", componentName.c_str(), ID);
        return World->AttachComponent(ID, componentName);
    }
}

EntityWrapper::operator EntityID()
{
    return this->ID;
}

