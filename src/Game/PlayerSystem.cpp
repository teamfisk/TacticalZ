#include "PlayerSystem.h"

void PlayerSystem::UpdateComponent(World* world, ComponentWrapper& player, double dt)
{
    player["Velocity"] = glm::vec3(0.f, 0.f, 0.f);
    if ((bool&)player["Forward"] == true) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt) * -1;

    }
    if ((bool&)player["Left"] == true) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt) * -1;
    }
    if ((bool&)player["Back"] == true) {
        ((glm::vec3&)player["Velocity"]).z = m_Speed * float(dt);
    }
    if ((bool&)player["Right"] == true) {
        ((glm::vec3&)player["Velocity"]).x = m_Speed * float(dt);
    }

    if ((glm::vec3)player["Velocity"] != glm::vec3(0.f)) {
        ComponentWrapper& transform = world->GetComponent(player.EntityID, "Transform");
        (glm::vec3&)transform["Position"] += (glm::vec3)player["Velocity"];
    }

    //decrease CoolDownTimers for both HeldItems
    ComponentWrapper& currentItem = world->GetComponent(player.EntityID, "PrimaryItem");
    ComponentWrapper& currentItem2 = world->GetComponent(player.EntityID, "SecondaryItem");
    currentItem["CoolDownTimer"] = std::max(0.0, (double)currentItem["CoolDownTimer"] - dt);
    currentItem2["CoolDownTimer"] = std::max(0.0, (double)currentItem2["CoolDownTimer"] - dt);

    //do shootEvent: if left mouse was released, and ammo/weaponcooldown/playeralive/shootingcooldown are ok
    if (m_LeftMouseWasReleased) {
        m_LeftMouseWasReleased = false;
        //get the health component linked to the playerId
        double currentHealth = (double)world->GetComponent(player.EntityID, "Health")["Health"];
        int currentAmmo = 0;
        double currentCoolDownTimer = 0.0;
        std::string heldItemString = "";
        if ((int)player["EquippedItem"] == (int)HeldItem::PrimaryItem)
            heldItemString = "PrimaryItem";
        if ((int)player["EquippedItem"] == (int)HeldItem::SecondaryItem)
            heldItemString = "SecondaryItem";

        if (heldItemString != "") {
            ComponentWrapper& currentItem = world->GetComponent(player.EntityID, heldItemString);
            currentAmmo = (int)currentItem["Ammo"];
            currentCoolDownTimer = (double)currentItem["CoolDownTimer"];

            if (currentHealth > 0.0 && currentAmmo > 0 && currentCoolDownTimer < 0.001) {
                //decrease ammo count
                //TODO: temp, set the cooldowntimer - probably done in some other system (itemSystem?) later
                currentItem["Ammo"] = (int)currentItem["Ammo"] - 1;
                currentItem["CoolDownTimer"] = 2.0;//change later! probably to maxCoolDownTimer
                //create and publish the shoot event
                Events::Shoot eShoot;
                eShoot.CurrentAimingPoint = m_AimingCoordinates;
                eShoot.CurrentlyEquippedItem = (int)(player["EquippedItem"]);
                m_EventBroker->Publish(eShoot);
            }
        }
    }
}

bool PlayerSystem::OnTouch(const Events::TriggerTouch &event)
{
    LOG_INFO("Player entity %i touched widget (entity %i).", event.Entity, event.Trigger);
    return false;
}

bool PlayerSystem::OnEnter(const Events::TriggerEnter &event)
{
    LOG_INFO("Player entity %i entered widget (entity %i).", event.Entity, event.Trigger);
    return false;
}

bool PlayerSystem::OnLeave(const Events::TriggerLeave &event)
{
    LOG_INFO("Player entity %i left widget (entity %i).", event.Entity, event.Trigger);
    return false;
}

bool PlayerSystem::OnMouseRelease(const Events::MouseRelease& e)
{
    //kolla ammoleft, cooldowntimer shooting
    //kolla om left mouse varit nere
    if (e.Button != GLFW_MOUSE_BUTTON_LEFT)
        return false;
    m_AimingCoordinates = glm::vec2(e.X, e.Y);
    m_LeftMouseWasReleased = true;
    return true;
}