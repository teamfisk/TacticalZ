#include "Systems/CapturePointSystem.h"
#include <algorithm>

CapturePointSystem::CapturePointSystem(EventBroker* eventBroker)
    : System(eventBroker),
    PureSystem("CapturePoint")
{
    //subscribe/listenTo playerdamage,healthpickup events (using the eventBroker)
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &CapturePointSystem::OnTriggerTouch);
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerLeave, &CapturePointSystem::OnTriggerLeave);
}

//here all capturepoints will update their component
//NOTE: needs to run each frame, since we're possibly modifying the captureTimer for the capturePoints by dt
void CapturePointSystem::UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& capturePoint, double dt)
{
    bool hasTeamComponent = world->HasComponent(capturePoint.EntityID, "Team");
    if (!hasTeamComponent) {
        world->AttachComponent(capturePoint.EntityID, "Team");
        ComponentWrapper& teamComponent = world->GetComponent(capturePoint.EntityID, "Team");
        teamComponent["Team"] = 0;
    }
    ComponentWrapper& teamComponent = world->GetComponent(capturePoint.EntityID, "Team");
    int firstTeamPlayersStandingInside = 0;
    int secondTeamPlayersStandingInside = 0;
    //what if capture point has no TEAM? -> NO ENUM.
    const int redTeam = (int)teamComponent["Team"].Enum("Red");//"team 1"
    const int blueTeam = (int)teamComponent["Team"].Enum("Blue");//"team 2"
    const int spectatorTeam = (int)teamComponent["Team"].Enum("Spectator");

    //check how many players are standing inside and are healthy
    for (size_t i = m_ETriggerTouchVector.size(); i > 0; i--)
    {
        auto triggerTouched = m_ETriggerTouchVector[i - 1];
        if (std::get<1>(triggerTouched) == capturePoint.EntityID) {
            //some player has touched this - lets figure out: what team, health
            EntityID playerID = std::get<0>(triggerTouched);
            if (!world->HasComponent(playerID, "Player")) {
                //if a non-player has entered the capturePoint, just erase that event and continue
                m_ETriggerTouchVector.erase(m_ETriggerTouchVector.begin() + i - 1);
                continue;
            }
            bool hasHealthComponent = world->HasComponent(playerID, "Health");
            if (hasHealthComponent) {
                double currentHealth = world->GetComponent(playerID, "Health")["Health"];
                //check if player is dead
                if ((int)currentHealth == 0) {
                    continue;
                }
            }
            //check team - spectatorNumber = "no team"
            int teamNumber = world->GetComponent(playerID, "Player")["Team"];
            if (teamNumber == redTeam) {
                firstTeamPlayersStandingInside++;
            } else if (teamNumber == blueTeam) {
                secondTeamPlayersStandingInside++;
            }
            continue;
        }
    }

    int ownedBy = teamComponent["Team"];

    //om ej next satt, förvänta sig att en capturepoint med en viss team färg kommer in...
    //sätt isåfall next och kör på..
    //gör inget tills man fått den infon

    /*check what capturePoint can be taken over next:
    no capturepoint taken yet for at least one of the teams <->
    at the start of the match the system is unaware of what capturePoint is the first one for each team*/
    if (m_Team1NextPossibleCapturePoint == m_NotACapturePoint && (int)teamComponent["Team"] == redTeam) {
        m_Team1NextPossibleCapturePoint = capturePoint["CapturePointNumber"];
        m_Team1HomeCapturePoint = capturePoint["CapturePointNumber"];//needed to calculate next m_Team1NextPossibleCapturePoint
    } else if (m_Team2NextPossibleCapturePoint == m_NotACapturePoint && (int)teamComponent["Team"] == blueTeam) {
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
        currentTeam = blueTeam;
    } else if (firstTeamPlayersStandingInside == 0 && secondTeamPlayersStandingInside > 0
        && m_Team2NextPossibleCapturePoint == (int)capturePoint["CapturePointNumber"])
    {
        timerDeltaChange = -secondTeamPlayersStandingInside*dt;
        currentTeam = redTeam;
    }

    if (firstTeamPlayersStandingInside == 0 && secondTeamPlayersStandingInside == 0) {
        //A.nobodys standing inside
        //do nothing (?)
    } else if (currentTeam == blueTeam || currentTeam == redTeam) {
        //B. at most one of the teams have players inside (this means datavariable currentTeam is not 0)
        //if capturePoint is not owned by the take-over team, just modify the CaptureTimer accordingly
        if (ownedBy != currentTeam) {
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] + timerDeltaChange;
        }
        //if capturePoint is owned by the team, and the other team has been trying to take it, then increase/decrease the timer towards 0.0
        if ((ownedBy == currentTeam && currentTeam == redTeam && (double)capturePoint["CaptureTimer"] < 0.0) ||
            (ownedBy == currentTeam && currentTeam == blueTeam && (double)capturePoint["CaptureTimer"] > 0.0)) {
            capturePoint["CaptureTimer"] = (double)capturePoint["CaptureTimer"] + timerDeltaChange;
        }
        //check if captureTimer > m_CaptureTimeToTakeOver and if so change owner and publish the eCaptured event
        if (abs((double)capturePoint["CaptureTimer"]) > abs(m_CaptureTimeToTakeOver)) {
            teamComponent["Team"] = currentTeam;
            capturePoint["CaptureTimer"] = 0.0;
            //publish Captured event
            Events::Captured e;
            e.CapturePointID = capturePoint.EntityID;
            e.TeamNumberThatCapturedCapturePoint = currentTeam;
            m_EventBroker->Publish(e);
            //modify nextPossibleCapturePoint, depending on, example: if team 1 has "0" as homebase or team 1 has "7" as homebase

            //0 = false 1 = true
            bool team1HasTheZeroCapturePoint = m_Team1HomeCapturePoint < m_Team2HomeCapturePoint;

            if (team1HasTheZeroCapturePoint) {
                if (currentTeam == redTeam) {
                    m_Team1NextPossibleCapturePoint++;
                } else {
                    m_Team2NextPossibleCapturePoint--;
                }
                //adjust flag for other team if their previous point has just been taken
                //this depends on what team has what homepoint ("side")
                if (m_Team2NextPossibleCapturePoint == m_Team1NextPossibleCapturePoint - 2) {
                    m_Team2NextPossibleCapturePoint = m_Team2NextPossibleCapturePoint + 1;
                }
                if (m_Team1NextPossibleCapturePoint == m_Team2NextPossibleCapturePoint + 2) {
                    m_Team1NextPossibleCapturePoint = m_Team1NextPossibleCapturePoint - 1;
                }
            } else {
                if (currentTeam == redTeam) {
                    m_Team1NextPossibleCapturePoint--;
                } else {
                    m_Team2NextPossibleCapturePoint++;
                }
                //adjust flag for other team if their previous point has just been taken
                //this depends on what team has what homepoint ("side")
                if (m_Team2NextPossibleCapturePoint == m_Team1NextPossibleCapturePoint + 2) {
                    m_Team2NextPossibleCapturePoint = m_Team2NextPossibleCapturePoint - 1;
                }
                if (m_Team1NextPossibleCapturePoint == m_Team2NextPossibleCapturePoint - 2) {
                    m_Team1NextPossibleCapturePoint = m_Team1NextPossibleCapturePoint + 1;
                }
            }
        }
    } else if (firstTeamPlayersStandingInside > 0 && secondTeamPlayersStandingInside > 0) {
        //C.both teams have players inside
        //do nothing (?)
    }

    //check for possible winCondition = check if the homebase is owned by the other team
    bool checkForWinner = false;
    if ((int)capturePoint["CapturePointNumber"] == m_Team1HomeCapturePoint && (int)teamComponent["Team"] != redTeam)
    {
        checkForWinner = true;
    }
    if ((int)capturePoint["CapturePointNumber"] == m_Team2HomeCapturePoint && (int)teamComponent["Team"] != blueTeam)
    {
        checkForWinner = true;
    }

    if (checkForWinner && !m_WinnerWasFound)
    {
        //publish Win event
        Events::Win e;
        e.TeamThatWon = teamComponent["Team"];
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
