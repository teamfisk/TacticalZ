#include "Game/Systems/TextFieldReader.h"

void TextFieldReader::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cAmmunitionHUD, double dt)
{
    if (!entity.HasComponent("Text")) {
        return;
    }

    // Find the entity to read from
    const std::string& parentEntityName = cAmmunitionHUD["ParentEntityName"];
    EntityWrapper readEntity = entity;
    if (!parentEntityName.empty()) {
        readEntity = entity.FirstParentByName(parentEntityName);
        if (!readEntity.Valid()) {
            return;
        }
    }

    // Find the component to read from
    const std::string& componentType = cAmmunitionHUD["ComponentType"];
    if (componentType.empty() || !readEntity.HasComponent(componentType)) {
        return;
    }
    ComponentWrapper component = readEntity[componentType];

    // Find the field to read from
    const std::string& fieldName = cAmmunitionHUD["Field"];
    if (fieldName.empty() || component.Info.Fields.count(fieldName) == 0) {
        return;
    }
    const ComponentInfo::Field_t& field = component.Info.Fields.at(fieldName);

    std::string& text = entity["Text"]["Content"];

    if (field.Type == "int") {
        text = boost::lexical_cast<std::string>((const int&)component[fieldName]);
    } else if (field.Type == "float") {
        text = boost::lexical_cast<std::string>((const float&)component[fieldName]);
    } else if (field.Type == "double") {
        text = boost::lexical_cast<std::string>((const double&)component[fieldName]);
    } else if (field.Type == "bool") {
        text = boost::lexical_cast<std::string>((const bool&)component[fieldName]);
    } else if (field.Type == "string") {
        text = (const std::string&)component[fieldName];
    }
}
