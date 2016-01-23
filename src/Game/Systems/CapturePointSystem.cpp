#include "Systems/CapturePointSystem.h"
#include <algorithm>

CapturePointSystem::CapturePointSystem(World* world, EventBroker* eventBroker) 
    : System(world, eventBroker)
    , PureSystem("CapturePoint")
{
    //subscribe/listenTo playerdamage,healthpickup events (using the eventBroker)
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &CapturePointSystem::OnTriggerTouch);
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerLeave, &CapturePointSystem::OnTriggerLeave);
    EVENT_SUBSCRIBE_MEMBER(m_ECaptured, &CapturePointSystem::OnCaptured);
}

//here all capturepoints will update their component
//NOTE: needs to run each frame, since we're possibly modifying the captureTimer for the capturePoints by dt
void CapturePointSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& capturePoint, double dt)
{
    if (m_WinnerWasFound) {
        return;
    }
    const int capturePointNumber = capturePoint["CapturePointNumber"];
    const bool hasTeamComponent = m_World->HasComponent(capturePoint.EntityID, "Team");

    //if point doesnt have a teamComponent yet, add one. since:
    //what if capture point has no team -> we cant get/use the team enum from it...
    if (!hasTeamComponent) {
        m_World->AttachComponent(capturePoint.EntityID, "Team");
        ComponentWrapper& teamComponent = m_World->GetComponent(capturePoint.EntityID, "Team");
        teamComponent["Team"] = (int)teamComponent["Team"].Enum("Spectator");
    }
    ComponentWrapper& teamComponent = m_World->GetComponent(capturePoint.EntityID, "Team");
    const int redTeam = (int)teamComponent["Team"].Enum("Red");
    const int blueTeam = (int)teamComponent["Team"].Enum("Blue");
    const int spectatorTeam = (int)teamComponent["Team"].Enum("Spectator");

    int homePointForTeam = (int)capturePoint["HomePointForTeam"];
    if (m_NumberOfCapturePoints == 0 && capturePointNumber != 0 && (homePointForTeam == redTeam || homePointForTeam == blueTeam)) {
        m_NumberOfCapturePoints = capturePointNumber + 1;//ex 2 -> 0,1,2 = 3
        if (homePointForTeam == redTeam) {
            m_RedTeamHomeCapturePoint = capturePointNumber;
            m_BlueTeamHomeCapturePoint = 0;
        } else {
            m_BlueTeamHomeCapturePoint = capturePointNumber;
            m_RedTeamHomeCapturePoint = 0;
        }
    }

    //if we havent received all capturepoints yet, just return
    if (m_NumberOfCapturePoints == 0 || m_NumberOfCapturePoints != m_CapturePointNumberToEntityIDMap.size()) {
        m_CapturePointNumberToEntityIDMap.insert(std::make_pair(capturePointNumber, capturePoint.EntityID));
        return;
    }

    //we have all capturepoints now - process stuff
    int ownedBy = teamComponent["Team"];
    int redTeamPlayersStandingInside = 0;
    int blueTeamPlayersStandingInside = 0;
    if (entity.HasComponent("Model")) {
        //Now sets team color to the capturepoint, or white if it is uncaptured.
        entity["Model"]["Color"] = ownedBy == blueTeam ? glm::vec4(0, 0.2f, 1, 1) : ownedBy == redTeam ? glm::vec4(1, 0.2f, 0, 1) : glm::vec4(1, 1, 1, 1);
    }

    //calculate next possible capturePoint for both teams
    std::map<std::string, int> nextPossibleCapturePoint;
    nextPossibleCapturePoint["Red"] = -1;
    nextPossibleCapturePoint["Blue"] = -1;
    for (size_t i = 0; i < m_NumberOfCapturePoints; i++)
    {
        if (!m_World->HasComponent(m_CapturePointNumberToEntityIDMap[i], "Team")) {
            continue;
        }
        ComponentWrapper& capturePointOwnedBy = m_World->GetComponent(m_CapturePointNumberToEntityIDMap[i], "Team");
        if ((int)capturePointOwnedBy["Team"] == redTeam && m_RedTeamHomeCapturePoint == 0) {
            nextPossibleCapturePoint["Red"] = i + 1;
        }
        if ((int)capturePointOwnedBy["Team"] == blueTeam && m_BlueTeamHomeCapturePoint == 0) {
            nextPossibleCapturePoint["Blue"] = i + 1;
        }
    }
    for (int i = m_NumberOfCapturePoints - 1; i >= 0; i--)
    {
        if (!m_World->HasComponent(m_CapturePointNumberToEntityIDMap[i], "Team")) {
            continue;
        }
        ComponentWrapper& capturePointOwnedBy = m_World->GetComponent(m_CapturePointNumberToEntityIDMap[i], "Team");
        if ((int)capturePointOwnedBy["Team"] == redTeam && m_RedTeamHomeCapturePoint != 0) {
            nextPossibleCapturePoint["Red"] = i - 1;
        }
        if ((int)capturePointOwnedBy["Team"] == blueTeam && m_BlueTeamHomeCapturePoint != 0) {
            nextPossibleCapturePoint["Blue"] = i - 1;
        }
    }

    //reset timers and reset the bool that triggers this
    if (m_ResetTimers) {
        for (size_t i = 0; i < m_NumberOfCapturePoints; i++)
        {
            ComponentWrapper& capturePoint = m_World->GetComponent(m_CapturePointNumberToEntityIDMap[i], "CapturePoint");
            if ((int)capturePoint["CapturePointNumber"] != nextPossibleCapturePoint["Red"] &&
                (int)capturePoint["CapturePointNumber"] != nextPossibleCapturePoint["Blue"]) {
                capturePoint["CaptureTimer"] = 0.0;
            }
        }
        m_ResetTimers = false;
    }

    //colorize next possible capturepoint
    if (nextPossibleCapturePoint["Red"] == capturePointNumber) {
        entity["Model"]["Color"] = glm::vec4(1, 1, 0, 1);
    }
    if (nextPossibleCapturePoint["Blue"] == capturePointNumber) {
        entity["Model"]["Color"] = glm::vec4(0, 1, 1, 1);
    }

    //check how many players are standing inside and are healthy
    for (size_t i = m_ETriggerTouchVector.size(); i > 0; i--)
    {
        auto triggerTouched = m_ETriggerTouchVector[i - 1];
        if (std::get<1>(triggerTouched) == capturePoint.EntityID) {
            //some player has touched this - lets figure out: what team, health
            EntityID playerID = std::get<0>(triggerTouched);
            if (!m_World->HasComponent(playerID, "Player")) {
                //if a non-player has entered the capturePoint, just erase that event and continue
                m_ETriggerTouchVector.erase(m_ETriggerTouchVector.begin() + i - 1);
                continue;
            }
            bool hasHealthComponent = m_World->HasComponent(playerID, "Health");
            if (hasHealthComponent) {
                double currentHealth = m_World->GetComponent(playerID, "Health")["Health"];
                //check if player is dead
                if ((int)currentHealth == 0) {
                    continue;
                }
            }
            //check team - spectatorNumber = "no team"
            int teamNumber = m_World->GetComponent(playerID, "Team")["Team"];
            if (teamNumber == redTeam) {
                redTeamPlayersStandingInside++;
            } else if (teamNumber == blueTeam) {
                blueTeamPlayersStandingInside++;
            }
            continue;
        }
    }

    //create data to be used in option B
    //check so this is the next possible capture point for the take-over team and see if only one team is standing inside it
    double timerDeltaChange = 0.0;
    int currentTeam = 0;
    bool canCapture = false;
    if (redTeamPlayersStandingInside > 0 && blueTeamPlayersStandingInside == 0) {
        timerDeltaChange = redTeamPlayersStandingInside*dt;
        currentTeam = redTeam;
        canCapture = nextPossibleCapturePoint["Red"] == capturePointNumber;
    }
    if (redTeamPlayersStandingInside == 0 && blueTeamPlayersStandingInside > 0) {
        timerDeltaChange = -blueTeamPlayersStandingInside*dt;
        currentTeam = blueTeam;
        canCapture = nextPossibleCapturePoint["Blue"] == capturePointNumber;
    }

    if (redTeamPlayersStandingInside == 0 && blueTeamPlayersStandingInside == 0) {
        //A.nobodys standing inside
        //do nothing (?)
    } else if (redTeamPlayersStandingInside > 0 && blueTeamPlayersStandingInside > 0) {
        //C.both teams have players inside
        //do nothing (?)
    } else {
        //B. at most one of the teams have players inside
        //if capturePoint is not owned by the take-over team, just modify the CaptureTimer accordingly
        if (ownedBy != currentTeam && canCapture) {
            if (abs((double)capturePoint["CaptureTimer"]) < 0.001f) {
                LOG_DEBUG("Point is being captured by team %i", currentTeam);   //Remove when we tested sufficiently.
            }
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] + timerDeltaChange;
        }
        //if capturePoint is owned by the team, and the other team has been trying to take it, then increase/decrease the timer towards 0.0
        if ((ownedBy == currentTeam && currentTeam == redTeam && (double)capturePoint["CaptureTimer"] < 0.0) ||
            (ownedBy == currentTeam && currentTeam == blueTeam && (double)capturePoint["CaptureTimer"] > 0.0)) {
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] + timerDeltaChange;
        }
        //check if captureTimer > m_CaptureTimeToTakeOver and if so change owner and publish the eCaptured event
        if (abs((double)capturePoint["CaptureTimer"]) > abs(m_CaptureTimeToTakeOver) && canCapture) {
            teamComponent["Team"] = currentTeam;
            capturePoint["CaptureTimer"] = 0.0;
            //publish Captured event
            LOG_DEBUG("Point is captured by team %i!", currentTeam);    //Remove when we tested sufficiently.
            Events::Captured e;
            e.CapturePointID = capturePoint.EntityID;
            e.TeamNumberThatCapturedCapturePoint = currentTeam;
            m_EventBroker->Publish(e);
            //NextPossibleCapturePoint will be calculated in the next update...
        }
    }

    //check for possible winCondition = check if the homebase is owned by the other team
    bool checkForWinner = false;
    if (capturePointNumber == m_RedTeamHomeCapturePoint && ownedBy != redTeam)
    {
        checkForWinner = true;
    }
    if (capturePointNumber == m_BlueTeamHomeCapturePoint && ownedBy != blueTeam)
    {
        checkForWinner = true;
    }

    if (checkForWinner && !m_WinnerWasFound)
    {
        //publish Win event
        Events::Win e;
        e.TeamThatWon = ownedBy;
        m_EventBroker->Publish(e);
        m_WinnerWasFound = true;
    }

}

bool CapturePointSystem::OnTriggerTouch(const Events::TriggerTouch& e)
{
    //personEntered = e.Entity, thingEntered = e.Trigger
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
bool CapturePointSystem::OnCaptured(const Events::Captured& e)
{
    //reset the timers in the next update since a capture has changed the "nextCapturePoint" for 1-2 teams
    m_ResetTimers = true;
    return true;
}
