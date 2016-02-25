#include "Systems/CapturePointSystem.h"
#include <algorithm>

CapturePointSystem::CapturePointSystem(SystemParams params)
    : System(params)
    , PureSystem("CapturePoint")
{
    //subscribe/listenTo playerdamage,healthpickup events (using the eventBroker)
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &CapturePointSystem::OnTriggerTouch);
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerLeave, &CapturePointSystem::OnTriggerLeave);
    EVENT_SUBSCRIBE_MEMBER(m_ECaptured, &CapturePointSystem::OnCaptured);

}

//here all capturepoints will update their component
//NOTE: needs to run each frame, since we're possibly modifying the captureTimer for the capturePoints by dt
void CapturePointSystem::UpdateComponent(EntityWrapper& capturePointEntity, ComponentWrapper& cCapturePoint, double dt)
{
    if (m_WinnerWasFound) {
        return;
    }
    const int capturePointNumber = cCapturePoint["CapturePointNumber"];
    const bool hasTeamComponent = capturePointEntity.HasComponent("Team");

    if (m_NumberOfCapturePoints != 0) {
        if (!m_CapturePointNumberToEntityMap[0].HasComponent("CapturePoint")) {
            //if map has changed, the capturepoints has changed, now have to redo them
            m_NumberOfCapturePoints = 0;
            m_CapturePointNumberToEntityMap.clear();
        }
    }
    //if point doesnt have a teamComponent yet, add one. since:
    //what if capture point has no team -> we cant get/use the team enum from it...
    if (!hasTeamComponent) {
        m_World->AttachComponent(cCapturePoint.EntityID, "Team");
        ComponentWrapper& teamComponent = capturePointEntity["Team"];
        teamComponent["Team"] = (int)teamComponent["Team"].Enum("Spectator");
    }
    ComponentWrapper& teamComponent = capturePointEntity["Team"];
    const int redTeam = (int)teamComponent["Team"].Enum("Red");
    const int blueTeam = (int)teamComponent["Team"].Enum("Blue");
    const int spectatorTeam = (int)teamComponent["Team"].Enum("Spectator");
    const double captureTimeToTakeOver = (double)cCapturePoint["CapturePointMaxTimer"];

    int homePointForTeam = (int)cCapturePoint["HomePointForTeam"];
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
    if (m_NumberOfCapturePoints == 0 || m_NumberOfCapturePoints != m_CapturePointNumberToEntityMap.size()) {
        m_CapturePointNumberToEntityMap.insert(std::make_pair(capturePointNumber, capturePointEntity));
        return;
    }

    //we have all capturepoints now - process stuff
    int ownedBy = teamComponent["Team"];
    int redTeamPlayersStandingInside = 0;
    int blueTeamPlayersStandingInside = 0;
    if (capturePointEntity.HasComponent("Model")) {
        //Now sets team color to the capturepoint, or white if it is uncaptured.
        capturePointEntity["Model"]["Color"] = ownedBy == blueTeam ? glm::vec4(0, 0.0f, 1, 0.3) : ownedBy == redTeam ? glm::vec4(1, 0.0f, 0, 0.3) : glm::vec4(1, 1, 1, 0.3);
    }

    //calculate next possible capturePoint for both teams
    std::map<std::string, int> nextPossibleCapturePoint;
    nextPossibleCapturePoint["Red"] = -1;
    nextPossibleCapturePoint["Blue"] = -1;
    for (int i = 0; i < m_NumberOfCapturePoints; i++) {
        if (!m_CapturePointNumberToEntityMap[i].HasComponent("Team")) {
            continue;
        }
        ComponentWrapper& capturePointOwnedBy = m_CapturePointNumberToEntityMap[i]["Team"];
        if ((int)capturePointOwnedBy["Team"] == redTeam && m_RedTeamHomeCapturePoint == 0) {
            nextPossibleCapturePoint["Red"] = i + 1;
        }
        if ((int)capturePointOwnedBy["Team"] == blueTeam && m_BlueTeamHomeCapturePoint == 0) {
            nextPossibleCapturePoint["Blue"] = i + 1;
        }
    }
    for (int i = m_NumberOfCapturePoints - 1; i >= 0; i--) {
        if (!m_CapturePointNumberToEntityMap[i].HasComponent("Team")) {
            continue;
        }
        ComponentWrapper& capturePointOwnedBy = m_CapturePointNumberToEntityMap[i]["Team"];
        if ((int)capturePointOwnedBy["Team"] == redTeam && m_RedTeamHomeCapturePoint != 0) {
            nextPossibleCapturePoint["Red"] = i - 1;
        }
        if ((int)capturePointOwnedBy["Team"] == blueTeam && m_BlueTeamHomeCapturePoint != 0) {
            nextPossibleCapturePoint["Blue"] = i - 1;
        }
    }

    //reset timers and reset the bool that triggers this
    if (m_ResetTimers) {
        for (int i = 0; i < m_NumberOfCapturePoints; i++) {
            ComponentWrapper& capturePoint = m_CapturePointNumberToEntityMap[i]["CapturePoint"];
            if ((int)capturePoint["CapturePointNumber"] != nextPossibleCapturePoint["Red"] &&
                (int)capturePoint["CapturePointNumber"] != nextPossibleCapturePoint["Blue"]) {
                //RED = +, BLUE = -, NONE
                auto teamOwners = (int)m_CapturePointNumberToEntityMap[i]["Team"]["Team"];
                if (teamOwners == redTeam || teamOwners == blueTeam) {
                    capturePoint["CaptureTimer"] = teamOwners == blueTeam ? -captureTimeToTakeOver : captureTimeToTakeOver;
                }
            }
        }
        m_ResetTimers = false;
    }

    //check how many players are standing inside and are healthy
    for (size_t i = m_ETriggerTouchVector.size(); i > 0; i--) {
        auto triggerTouched = m_ETriggerTouchVector[i - 1];
        if (std::get<1>(triggerTouched) == capturePointEntity) {
            //some player has touched this - lets figure out: what team, health
            EntityWrapper player = std::get<0>(triggerTouched);
            //check if its really a player that has triggered the touch
            if (!player.HasComponent("Player")) {
                //if a non-player has entered the capturePoint, just erase that event and continue
                m_ETriggerTouchVector.erase(m_ETriggerTouchVector.begin() + i - 1);
                continue;
            }
            bool hasHealthComponent = player.HasComponent("Health");
            if (hasHealthComponent) {
                double currentHealth = player["Health"]["Health"];
                //check if player is dead
                if ((int)currentHealth == 0) {
                    continue;
                }
            }
            //check team - spectatorNumber = "no team"
            int teamNumber = player["Team"]["Team"];
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
        timerDeltaChange = 3*redTeamPlayersStandingInside*dt;
        currentTeam = redTeam;
        canCapture = nextPossibleCapturePoint["Red"] == capturePointNumber;
    }
    if (redTeamPlayersStandingInside == 0 && blueTeamPlayersStandingInside > 0) {
        timerDeltaChange = -3*blueTeamPlayersStandingInside*dt;
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
            cCapturePoint["CaptureTimer"] = (double)cCapturePoint["CaptureTimer"] + timerDeltaChange;
        }
        //if capturePoint is owned by the team, and the other team has been trying to take it, then increase/decrease the timer towards 0.0
        if ((ownedBy == currentTeam && currentTeam == redTeam && (double)cCapturePoint["CaptureTimer"] < captureTimeToTakeOver) ||
            (ownedBy == currentTeam && currentTeam == blueTeam && (double)cCapturePoint["CaptureTimer"] > -captureTimeToTakeOver)) {
            cCapturePoint["CaptureTimer"] = (double)cCapturePoint["CaptureTimer"] + timerDeltaChange;
        }
        //check if captureTimer > captureTimeToTakeOver and if so change owner and publish the eCaptured event
        if (abs((double)cCapturePoint["CaptureTimer"]) > captureTimeToTakeOver && canCapture) {
            teamComponent["Team"] = currentTeam;
            cCapturePoint["CaptureTimer"] = glm::sign((double)cCapturePoint["CaptureTimer"])*captureTimeToTakeOver;
            //publish Captured event
            Events::Captured e;
            e.CapturePointID = cCapturePoint.EntityID;
            e.TeamNumberThatCapturedCapturePoint = currentTeam;
            m_EventBroker->Publish(e);
            //NextPossibleCapturePoint will be calculated in the next update...
        }
    }

    //check for possible winCondition = check if the homebase is owned by the other team
    bool checkForWinner = false;
    if (capturePointNumber == m_RedTeamHomeCapturePoint && ownedBy != redTeam) {
        checkForWinner = true;
    }
    if (capturePointNumber == m_BlueTeamHomeCapturePoint && ownedBy != blueTeam) {
        checkForWinner = true;
    }

    if (checkForWinner && !m_WinnerWasFound) {
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
    for (size_t i = 0; i < m_ETriggerTouchVector.size(); i++) {
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
