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
//NOTE: needs to run each frame, since we're possibly modifying the captureTimer for the capturePoints by dt
void CapturePointSystem::UpdateComponent(World *world, ComponentWrapper &capturePoint, double dt)
{
    //for testing only:
    //dt = 20.0;
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

    //check what capturePoint can be taken over next
    //TODO: modify this when a point has been taken over
    //A. no capturepoint taken yet for at least one of the teams
    //A1. at the start of the match the system is unaware of what capturePoint is the first one for each team
    if (m_Team1NextPossibleCapturePoint == m_NotACapturePoint && (int)capturePoint["IsHomeCapturePointForTeamNumber"] == 1) {
        m_Team1NextPossibleCapturePoint = capturePoint["CapturePointNumber"];
        m_Team1HomeCapturePoint = capturePoint["CapturePointNumber"];//needed to calculate next m_Team1NextPossibleCapturePoint
    }
    if (m_Team2NextPossibleCapturePoint == m_NotACapturePoint && (int)capturePoint["IsHomeCapturePointForTeamNumber"] == 2) {
        m_Team2NextPossibleCapturePoint = capturePoint["CapturePointNumber"];
        m_Team2HomeCapturePoint = capturePoint["CapturePointNumber"];//needed to calculate next m_Team2NextPossibleCapturePoint
    }
    //B. at least one capturepoint has been taken over
    //do nothing, its being handled inside the next code:

    //TODO: refactor code a bit

    //A.nobodys standing inside
    if (firstTeamPlayersStandingInside == 0 && secondTeamPlayersStandingInside == 0) {
        //do nothing (?)
    }
    //B.first team has players but second none, and this capturePoint is the next in line to be able to be captured
    if (firstTeamPlayersStandingInside > 0 && secondTeamPlayersStandingInside == 0
        && m_Team1NextPossibleCapturePoint == (int)capturePoint["CapturePointNumber"]) {
        //ownedBy 1 -> timer should stay at 15
        //increased by numberOfPlayersInside*dt
        if (ownedBy == 2 || ownedBy == 0)
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] + firstTeamPlayersStandingInside*dt;
        //check if captureTimer > 15 and if so change owner and publish the eCaptured event
        //TODO: graphics 25,50,75% captured events? for graphical issues
        if ((double)capturePoint["CaptureTimer"] > 15.0) {
            //publish Captured event
            capturePoint["OwnedBy"] = 1;
            Events::Captured e;
            e.CapturePointID = capturePoint.EntityID;
            e.TeamNumberThatCapturedCapturePoint = 1;
            m_EventBroker->Publish(e);
            //modify next m_Team1NextPossibleCapturePoint
            //example team1:s homepoint is at 0 and team2:s at 7. team 1 capture 3, next will be 4
            //example team1:s homepoint is at 7 and team2:s at 0. team 1 capture 3, next will be 2
            if (m_Team1HomeCapturePoint < m_Team2HomeCapturePoint) {
                m_Team1NextPossibleCapturePoint++;
                //if this was a contested capturePoint (i.e. both teams try to take point 3), then modify other teams next point as well
                if (m_Team1NextPossibleCapturePoint > m_Team2NextPossibleCapturePoint)
                    m_Team2NextPossibleCapturePoint = m_Team1NextPossibleCapturePoint;
            }
            else
            {
                m_Team1NextPossibleCapturePoint--;
                //if this was a contested capturePoint (i.e. both teams try to take point 3), then modify other teams next point as well
                if (m_Team1NextPossibleCapturePoint < m_Team2NextPossibleCapturePoint)
                    m_Team2NextPossibleCapturePoint = m_Team1NextPossibleCapturePoint;
            }
        }
    }
    //C.second team has players but second none, and this capturePoint is the next in line to be able to be captured
    if (firstTeamPlayersStandingInside == 0 && secondTeamPlayersStandingInside > 0
        && m_Team2NextPossibleCapturePoint == (int)capturePoint["CapturePointNumber"]) {
        //ownedBy 2 -> timer should stay at -15
        //decreased by numberOfPlayersInside*dt
        if (ownedBy == 1 || ownedBy == 0)
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] - secondTeamPlayersStandingInside*dt;
        //check if captureTimer < -15 and if so change owner and publish the eCaptured event
        //TODO: graphics 25,50,75% captured events? for graphical issues
        if ((double)capturePoint["CaptureTimer"] < -15.0) {
            capturePoint["OwnedBy"] = 2;
            Events::Captured e;
            e.CapturePointID = capturePoint.EntityID;
            e.TeamNumberThatCapturedCapturePoint = 2;
            m_EventBroker->Publish(e);
            //modify next m_Team2NextPossibleCapturePoint
            //example team2:s homepoint is at 0 and team1:s at 7. team 2 capture 3, next will be 4
            //example team2:s homepoint is at 7 and team1:s at 0. team 2 capture 3, next will be 2
            if (m_Team2HomeCapturePoint < m_Team1HomeCapturePoint)
            {
                m_Team2NextPossibleCapturePoint++;
                //if this was a contested capturePoint (i.e. both teams try to take point 3), then modify other teams next point as well
                if (m_Team2NextPossibleCapturePoint > m_Team1NextPossibleCapturePoint)
                    m_Team1NextPossibleCapturePoint = m_Team2NextPossibleCapturePoint;
            }
            else
            {
                m_Team2NextPossibleCapturePoint--;
                //if this was a contested capturePoint (i.e. both teams try to take point 3), then modify other teams next point as well
                if (m_Team2NextPossibleCapturePoint < m_Team1NextPossibleCapturePoint)
                    m_Team1NextPossibleCapturePoint = m_Team2NextPossibleCapturePoint;
            }
        }

    }
    //D.both teams have players inside
    if (firstTeamPlayersStandingInside > 0 && secondTeamPlayersStandingInside > 0) {
        //do nothing (?)
    }

    //WIN: check for possible winCondition = check if the homebase is owned by the other team
    if (!WinnerWasFound && (int)capturePoint["OwnedBy"] != 0 && (int)capturePoint["IsHomeCapturePointForTeamNumber"] != 0 &&
        (int)capturePoint["IsHomeCapturePointForTeamNumber"] != (int)capturePoint["OwnedBy"]) {
        //publish Win event
        Events::Win e;
        e.TeamThatWon = capturePoint["OwnedBy"];
        m_EventBroker->Publish(e);
        WinnerWasFound = true;
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
