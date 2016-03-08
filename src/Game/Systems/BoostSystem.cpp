#include "Systems/BoostSystem.h"

BoostSystem::BoostSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &BoostSystem::OnPlayerDamage);
}

bool BoostSystem::OnPlayerDamage(Events::PlayerDamage& e)
{
    if (e.Victim.ID == e.Inflictor.ID) {
        return false;
    }

    if (!e.Victim.Valid() || !LocalPlayer.Valid()) {
        return false;
    }

    if (e.Victim != LocalPlayer && !e.Victim.IsChildOf(LocalPlayer)) {
        return false;
    }

    if (!e.Inflictor.Valid()) {
        return false;
    }

    //if its not friendly fire, return
    if ((int)m_World->GetComponent(e.Inflictor.ID, "Team")["Team"] != (int)m_World->GetComponent(e.Victim.ID, "Team")["Team"]) {
        return false;
    }

    //determine the inflictors class
    auto className = DetermineClass(e.Inflictor);
    if (className == "") {
        return false;
    }

    //get the XML file, example: "Schema/Entities/BoostclassName.xml"
    std::string classXML = "Schema/Entities/" + className + ".xml";

    //check if player already has a child with the component, if so delete that child
    auto playerBoostAssaultEntity = e.Victim.FirstChildByName(className);
    if (playerBoostAssaultEntity.Valid()) {
        m_World->DeleteEntity(playerBoostAssaultEntity.ID);
    }
    //load boost XML file, set it entity parented with the victim player
    auto entityFile = ResourceManager::Load<EntityFile>(classXML);
    EntityWrapper boostAssaultEntity = entityFile->MergeInto(m_World);
    m_World->SetName(boostAssaultEntity.ID, className);
    m_World->SetParent(boostAssaultEntity.ID, e.Victim.ID);

    return true;
}

std::string BoostSystem::DetermineClass(EntityWrapper inflictorPlayer)
{
    //determine the class based on what component the inflictor-player has
    if (m_World->HasComponent(inflictorPlayer.ID, "DashAbility")) {
        return "BoostAssault";
    }
    if (m_World->HasComponent(inflictorPlayer.ID, "ShieldAbility")) {
        return "BoostDefender";
    }
    if (m_World->HasComponent(inflictorPlayer.ID, "SprintAbility")) {
        return "BoostSniper";
    }
    return "";
}
