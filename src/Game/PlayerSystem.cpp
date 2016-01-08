#include "PlayerSystem.h"

void PlayerSystem::UpdateComponent(World * world, ComponentWrapper & player, double dt)
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

    //do shootEvent: if left mouse was released, and ammo/weaponcooldown/playeralive/shootingcooldown are ok
    if (leftMouseWasReleased) {
        leftMouseWasReleased = false;
        //get the health component linked to the playerId
        double currentHealth = (double)world->GetComponent(player.EntityID, "Health")["Health"];
        int currentAmmo = 0;
        double currentCoolDownTimer = 0.0f;

        if ((int)player["EquippedItem"] == 1) {
            ComponentWrapper& currentItem = world->GetComponent(player.EntityID, "PrimaryItem");
            currentAmmo = currentItem["Ammo"];
            //subtract 1 ammo - the ItemSystem will probably listen to eShoot and handle the CoolDownTimer
            currentItem["Ammo"] = (int)currentItem["Ammo"] -1;
            int test = (int)currentItem["Ammo"];
            currentCoolDownTimer = (double)world->GetComponent(player.EntityID, "PrimaryItem")["CoolDownTimer"];
        }
        if ((int)player["EquippedItem"] == 2) {
            ComponentWrapper& currentItem = world->GetComponent(player.EntityID, "SecondaryItem");
            currentAmmo = currentItem["Ammo"];
            //subtract 1 ammo - the ItemSystem will probably listen to eShoot and handle the CoolDownTimer
            currentItem["Ammo"] = (int)currentItem["Ammo"] - 1;
            currentCoolDownTimer = (double)world->GetComponent(player.EntityID, "SecondaryItem")["CoolDownTimer"];
        }
        if (currentHealth > 0.0f && currentAmmo > 0 && currentCoolDownTimer < 0.001f) {
            //create and publish the shoot event
            Events::Shoot eShoot;
            eShoot.currentAimingPoint = aimingCoordinates;
            eShoot.weaponType = (int)player["EquippedItem"];
            m_EventBroker->Publish(eShoot);
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
    aimingCoordinates = glm::vec2(e.X, e.Y);
    leftMouseWasReleased = true;
    return true;
}