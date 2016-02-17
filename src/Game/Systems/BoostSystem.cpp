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

    //friendly fire

    auto className = determineClass(e.Inflictor);
    if (className == "") {
        return false;
    }
    //"Schema/Entities/BoostAssault.xml"
    std::string classXML = "Schema/Entities/" + className + ".xml";

    //class

    //load & set the BoostAssault Component
    auto entityFile = ResourceManager::Load<EntityFile>(classXML);
    EntityFileParser parser(entityFile);
    EntityID boostAssaultEntity = parser.MergeEntities(m_World);
    m_World->SetParent(boostAssaultEntity, e.Victim.ID);

    return true;
}

std::string BoostSystem::determineClass(EntityWrapper player)
{
    if (m_World->HasComponent(player.ID, "DashAbility")) {
        return "BoostAssault";
    }
    if (m_World->HasComponent(player.ID, "DefenderShield")) {
        return "BoostDefender";
    }
    if (m_World->HasComponent(player.ID, "SniperSprint")) {
        return "BoostSniper";
    }
    return "";
}
