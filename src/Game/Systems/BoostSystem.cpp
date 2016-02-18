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
    if (!e.Victim.Valid() && e.Victim != LocalPlayer && !e.Victim.IsChildOf(LocalPlayer)) {
        return false;
    }

    if (!e.Inflictor.Valid() || !e.Victim.Valid()) {
        return false;
    }

    auto teamInflictor = m_World->GetComponent(e.Inflictor.ID, "Team");
    auto teamVictim = m_World->GetComponent(e.Victim.ID, "Team");
    if ((int)teamInflictor["Team"] != (int)teamVictim["Team"]) {
        return false;
    }

    auto className = DetermineClass(e.Inflictor);
    if (className == "") {
        return false;
    }
    //"Schema/Entities/Boost-.xml"
    std::string classXML = "Schema/Entities/" + className + ".xml";

    //check if player already has the component
    auto playerBoostAssaultEntity = e.Victim.FirstChildByName(className);
    if (playerBoostAssaultEntity.Valid()) {
        m_World->DeleteEntity(playerBoostAssaultEntity.ID);
    }
    //load & set the BoostAssault Component
    auto entityFile = ResourceManager::Load<EntityFile>(classXML);
    EntityFileParser parser(entityFile);
    EntityID boostAssaultEntity = parser.MergeEntities(m_World);
    m_World->SetName(boostAssaultEntity, className);
    m_World->SetParent(boostAssaultEntity, e.Victim.ID);

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
