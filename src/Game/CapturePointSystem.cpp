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
void CapturePointSystem::UpdateComponent(World* world, ComponentWrapper& capturePoint, double dt)
{
    int firstTeamPlayersStandingInside = 0;
    int secondTeamPlayersStandingInside = 0;

    //check how many players are standing inside and are healthy
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
            int teamNumber = world->GetComponent(playerID, "Player")["TeamNumber"];
            if (teamNumber == 1)
                firstTeamPlayersStandingInside++;
            else if (teamNumber == 2)
                secondTeamPlayersStandingInside++;
            continue;
        }
    }

    int ownedBy = capturePoint["OwnedBy"];

    /*check what capturePoint can be taken over next:
    no capturepoint taken yet for at least one of the teams <->
    at the start of the match the system is unaware of what capturePoint is the first one for each team*/
    if (m_Team1NextPossibleCapturePoint == m_NotACapturePoint && (int)capturePoint["IsHomeCapturePointForTeamNumber"] == 1) {
        m_Team1NextPossibleCapturePoint = capturePoint["CapturePointNumber"];
        m_Team1HomeCapturePoint = capturePoint["CapturePointNumber"];//needed to calculate next m_Team1NextPossibleCapturePoint
    }
    else if (m_Team2NextPossibleCapturePoint == m_NotACapturePoint && (int)capturePoint["IsHomeCapturePointForTeamNumber"] == 2) {
        m_Team2NextPossibleCapturePoint = capturePoint["CapturePointNumber"];
        m_Team2HomeCapturePoint = capturePoint["CapturePointNumber"];//needed to calculate next m_Team2NextPossibleCapturePoint
    }
    //at least one capturepoint has been taken over
    //do nothing, its being handled inside the next code:

    //create data to be used in option B
    //check so this is the next possible capture point for the take-over team and see if only one team is standing inside it
    double timerDeltaChange = 0.0;
    int currentTeam = 0;
    if (firstTeamPlayersStandingInside > 0 && secondTeamPlayersStandingInside == 0
        && m_Team1NextPossibleCapturePoint == (int)capturePoint["CapturePointNumber"])
    {
        timerDeltaChange = firstTeamPlayersStandingInside*dt;
        currentTeam = 1;
    }
    else if (firstTeamPlayersStandingInside == 0 && secondTeamPlayersStandingInside > 0
        && m_Team2NextPossibleCapturePoint == (int)capturePoint["CapturePointNumber"])
    {
        timerDeltaChange = -secondTeamPlayersStandingInside*dt;
        currentTeam = 2;
    }

    //A.nobodys standing inside
    if (firstTeamPlayersStandingInside == 0 && secondTeamPlayersStandingInside == 0) {
        //do nothing (?)
    }

    //B. at most one of the teams have players inside (this means datavariable currentTeam is not 0)
    else if (currentTeam != 0) {
        //if capturePoint is not owned by the take-over team, just modify the CaptureTimer accordingly
        if (ownedBy != currentTeam)
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] + timerDeltaChange;
        //if capturePoint is owned by the team, and the other team has been trying to take it, then increase/decrease the timer towards 0.0
        if (ownedBy == currentTeam && currentTeam == 1 && (double)capturePoint["CaptureTimer"] < 0.0)
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] + timerDeltaChange;
        if (ownedBy == currentTeam && currentTeam == 2 && (double)capturePoint["CaptureTimer"] > 0.0)
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] + timerDeltaChange;
        //check if captureTimer > m_CaptureTimeToTakeOver and if so change owner and publish the eCaptured event
        if (abs((double)capturePoint["CaptureTimer"]) > abs(m_CaptureTimeToTakeOver)) {
            capturePoint["OwnedBy"] = currentTeam;
            capturePoint["CaptureTimer"] = 0.0;
            //publish Captured event
            Events::Captured e;
            e.CapturePointID = capturePoint.EntityID;
            e.TeamNumberThatCapturedCapturePoint = currentTeam;
            m_EventBroker->Publish(e);
            //modify nextPossibleCapturePoint, depending on, example: if team 1 has "0" as homebase or team 1 has "7" as homebase
            if (m_Team1HomeCapturePoint < m_Team2HomeCapturePoint) {
                if (currentTeam == 1) 
                    m_Team1NextPossibleCapturePoint++;
                if (currentTeam == 2)
                    m_Team2NextPossibleCapturePoint--;
                //if this was a contested capturePoint (i.e. both teams try to take point 3), then modify other teams next point as well
                if (m_Team1NextPossibleCapturePoint > m_Team2NextPossibleCapturePoint)
                    m_Team2NextPossibleCapturePoint = m_Team1NextPossibleCapturePoint;
            }
            else
            {
                if (currentTeam == 1)
                    m_Team1NextPossibleCapturePoint--;
                if (currentTeam == 2)
                    m_Team2NextPossibleCapturePoint++;
                //if this was a contested capturePoint (i.e. both teams try to take point 3), then modify other teams next point as well
                if (m_Team1NextPossibleCapturePoint < m_Team2NextPossibleCapturePoint)
                    m_Team2NextPossibleCapturePoint = m_Team1NextPossibleCapturePoint;
            }
        }
    }

    //C.both teams have players inside
    else if (firstTeamPlayersStandingInside > 0 && secondTeamPlayersStandingInside > 0) {
        //do nothing (?)
    }

    //check for possible winCondition = check if the homebase is owned by the other team
    if (!m_WinnerWasFound && (int)capturePoint["OwnedBy"] != 0 && (int)capturePoint["IsHomeCapturePointForTeamNumber"] != 0 &&
        (int)capturePoint["IsHomeCapturePointForTeamNumber"] != (int)capturePoint["OwnedBy"]) {
        //publish Win event
        Events::Win e;
        e.TeamThatWon = capturePoint["OwnedBy"];
        m_EventBroker->Publish(e);
        m_WinnerWasFound = true;
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
