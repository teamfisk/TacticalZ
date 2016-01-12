#include "CapturePointSystem.h"
#include <algorithm>

CapturePointSystem::CapturePointSystem(EventBroker* eventBroker)
    : PureSystem(eventBroker, "CapturePoint")
{
    //subscribe/listenTo playerdamage,healthpickup events (using the eventBroker)
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &CapturePointSystem::OnTriggerTouch);
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerLeave, &CapturePointSystem::OnTriggerLeave);
}

//here all capturepoints will update their component
void CapturePointSystem::UpdateComponent(World *world, ComponentWrapper &capturePoint, double dt)
{
    //NOTE: needs to run each frame, since we're possibly increasing the captureTimer for the capturePoint by dt
    int firstTeamPlayersStandingInside = 0;
    int secondTeamPlayersStandingInside = 0;

    for (size_t i = 0; i < m_ETriggerTouchVector.size(); i++)
    {
        auto triggerTouched = m_ETriggerTouchVector[i];
        if (std::get<1>(triggerTouched) == capturePoint.EntityID) {
            //some player has touched this - lets figure out: what team, health
            EntityID playerID = std::get<0>(triggerTouched);
            bool hasHealthComponent = world->HasComponent(playerID, "Health");
            if (!hasHealthComponent)
                continue;
            double currentHealth = world->GetComponent(playerID, "Health")["Health"];
            //check if player is dead
            if ((int)currentHealth == 0)
                continue;
            //check team - 0 = no team
            int teamNumber = (int)world->GetComponent(playerID, "Player")["TeamNumber"];
            if (teamNumber == 1)
                firstTeamPlayersStandingInside++;
            if (teamNumber == 2)
                secondTeamPlayersStandingInside++;
            continue;
        }
    }

    int ownedBy = capturePoint["OwnedBy"];
    double captureTimer = capturePoint["CaptureTimer"];

    //A.nobodys standing inside
    if (firstTeamPlayersStandingInside == 0 && secondTeamPlayersStandingInside == 0) {
        //do nothing (?)
    }
    //B.first team has players but second none
    if (firstTeamPlayersStandingInside > 0 && secondTeamPlayersStandingInside == 0) {
        if (ownedBy == 2 || ownedBy == 0)
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] + dt;
        //check if captureTimer > 5 and if so change owner
        if ((double)capturePoint["CaptureTimer"] > 5.0) {
            capturePoint["OwnedBy"] = 1;
            capturePoint["CaptureTimer"] = 0;
        }
    }
    //C.second team has players but second none
    if (firstTeamPlayersStandingInside == 0 && secondTeamPlayersStandingInside > 0) {

    }
    //D.both teams have players inside
    if (firstTeamPlayersStandingInside > 0 && secondTeamPlayersStandingInside > 0) {

    }



}

bool CapturePointSystem::OnTriggerTouch(const Events::TriggerTouch& e)
{
    //auto personEntered = e.Entity;
    //auto thingEntered = e.Trigger;
    m_ETriggerTouchVector.push_back(std::make_tuple(e.Entity, e.Trigger));
    return true;
}

bool CapturePointSystem::OnTriggerLeave(const Events::TriggerLeave& e)
{
    for (size_t i = 0; i < m_ETriggerTouchVector.size(); i++)
    {
        auto triggerTouched = m_ETriggerTouchVector[i];
        if (std::get<0>(triggerTouched) == e.Entity && std::get<1>(triggerTouched) == e.Trigger) {
            m_ETriggerTouchVector.erase(m_ETriggerTouchVector.begin() + i);
            break;
        }
    }
    return true;
}
