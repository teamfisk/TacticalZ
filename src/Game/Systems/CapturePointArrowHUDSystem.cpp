#include "Systems/CapturePointArrowHUDSystem.h"

CapturePointArrowHUDSystem::CapturePointArrowHUDSystem(SystemParams params)
    : System(params)
    , ImpureSystem()
{
    EVENT_SUBSCRIBE_MEMBER(m_ECapturedEvent, &CapturePointArrowHUDSystem::OnCapturePointCaptured);
}


void CapturePointArrowHUDSystem::Update(double dt)
{
    bool loadCheck = true;
    int redTeamEnum;
    int blueTeamEnum;
    int spectatorTeamEnum;

    //Get list for all CapturePointArrowHUDComponents
    auto arrowHUDs = m_World->GetComponents("CapturePointArrowHUD");
    auto capturePoints = m_World->GetComponents("CapturePoint");
    if(arrowHUDs == nullptr) {
        return;
    }

    for(auto& cArrowHUD : *arrowHUDs) {
        //Get what team the current arrow corresponds to
        EntityWrapper arrowEntity = EntityWrapper(m_World, cArrowHUD.EntityID);
        if (!arrowEntity.Valid()) {
            continue;
        }
        if(!arrowEntity.HasComponent("Team")) {
            continue;
        }
        auto cTeam = arrowEntity["Team"];
        int currentTeam = (int)cTeam["Team"];

        if (loadCheck) {
            redTeamEnum = (int)cTeam["Team"].Enum("Red");
            blueTeamEnum = (int)cTeam["Team"].Enum("Blue");
            spectatorTeamEnum = (int)cTeam["Team"].Enum("Spectator");
            loadCheck = false;


            if (!m_InitialtargetsSet) {
                std::unordered_map<int, glm::vec3> blueTargets, redTargets;
                EntityWrapper homeBlue, homeRed;
                int lastCP = -INFINITY;
                int firstCP = INFINITY;

                for (auto& cCP : *capturePoints) {
                    auto homePointTeam = (int)cCP["HomePointForTeam"];
                    EntityWrapper capturePointEntity = EntityWrapper(m_World, cCP.EntityID);
                    int capturePointID = (int)capturePointEntity["CapturePoint"]["CapturePointNumber"];

                    if(capturePointID < firstCP) {
                        firstCP = capturePointID;
                    }

                    if(capturePointID > lastCP) {
                        lastCP = capturePointID;
                    }

                    if (!capturePointEntity.HasComponent("Team")) {
                        continue;
                    }

                    int currentOwner = (int)capturePointEntity["Team"]["Team"];

                    if(currentOwner != redTeamEnum) {
                        //This capturePoint is not owned by the red team and is therefor an eligible target for red team
                        glm::vec3 targetPos = Transform::AbsolutePosition(capturePointEntity);
                        redTargets.insert(std::pair<int, glm::vec3>(capturePointID, targetPos));
                    }
                    if(currentOwner != blueTeamEnum) {
                        //This capturePoint is not owned by the blue team and is therefor an eligible target for blue team
                        glm::vec3 targetPos = Transform::AbsolutePosition(capturePointEntity);
                        blueTargets.insert(std::pair<int, glm::vec3>(capturePointID, targetPos));
                    }

                    if(homePointTeam == blueTeamEnum) {
                        //CP is the home point for blue team.
                        homeBlue = capturePointEntity;
                    } else if (homePointTeam == redTeamEnum) {
                        //CP is the home point for red team.
                        homeRed = capturePointEntity;
                    }
                }

                if(!homeRed.Valid() || !homeBlue.Valid()) {
                    //One or both teams have no home point, cant continue
                    return;
                }

                std::unordered_map<int, glm::vec3>::const_iterator got;
                //Find next target for red team.
                if((int)homeRed["CapturePoint"]["CapturePointNumber"] == lastCP) {
                    //Red home base is the last capture point, count back from lastCP and find next target
                    for (int i = lastCP; i >= firstCP; i--) {
                        got = redTargets.find(i);
                        if(got == redTargets.end()) {
                            continue;
                        } else {
                            m_RedTeamCurrentTarget = got->second;
                            break;
                        }
                    }
                } else if ((int)homeRed["CapturePoint"]["CapturePointNumber"] == firstCP) {
                    //Red home is the first capture point, count forward from firstCP and find next target.
                    for (int i = firstCP; i <= lastCP; i++) {
                        got = redTargets.find(i);
                        if(got == redTargets.end()) {
                            //Target was not found, try the next one after that.
                            continue;
                        } else {
                            m_RedTeamCurrentTarget = got->second;
                            break;
                        }
                    }
                }

                //Find next target for blue team
                if ((int)homeBlue["CapturePoint"]["CapturePointNumber"] == lastCP) {
                    //Red home base is the last capture point, count back from lastCP and find next target
                    for (int i = lastCP; i >= firstCP; i--) {
                        got = blueTargets.find(i);
                        if (got == blueTargets.end()) {
                            continue;
                        } else {
                            m_BlueTeamCurrentTarget = got->second;
                            break;
                        }
                    }
                } else if ((int)homeBlue["CapturePoint"]["CapturePointNumber"] == firstCP) {
                    //Red home is the first capture point, count forward from firstCP and find next target.
                    for (int i = firstCP; i <= lastCP; i++) {
                        got = blueTargets.find(i);
                        if (got == blueTargets.end()) {
                            //Target was not found, try the next one after that.
                            continue;
                        } else {
                            m_BlueTeamCurrentTarget = got->second;
                            break;
                        }
                    }
                }
            }
        }
        //if red team, get red team next point, otherwise blue team next point.
        //Untill this is awailable we will just use the hardcoded value in the component.
        //This will also give us a position, so we wont need to loop through all capturePoints.
        glm::vec3 pos;
        if(currentTeam == redTeamEnum) {
            pos = m_RedTeamCurrentTarget;
        } else if (currentTeam == blueTeamEnum) {
            pos = m_BlueTeamCurrentTarget;
        }

        glm::vec3& arrowOri = arrowEntity["Transform"]["Orientation"];
        glm::vec3 lookVector = glm::normalize(Transform::AbsolutePosition(arrowEntity) - pos); //Maybe should be player instead
        float pitch = std::asin(-lookVector.y);
        float yaw = std::atan2(lookVector.x, lookVector.z);
        arrowOri.x = pitch;
        arrowOri.y = yaw;
        arrowOri.z = 0.f;
        EntityWrapper parent = arrowEntity.Parent();
        if (parent.Valid()) {
            arrowOri -= Transform::AbsoluteOrientationEuler(parent);
        }
    }
}

bool CapturePointArrowHUDSystem::OnCapturePointCaptured(Events::Captured& e)
{
    if(!e.BlueTeamNextCapturePoint.Valid() || !e.RedTeamNextCapturePoint.Valid()) {
        return 0;
    }

    m_RedTeamCurrentTarget = Transform::AbsolutePosition(e.RedTeamNextCapturePoint);
    m_BlueTeamCurrentTarget = Transform::AbsolutePosition(e.BlueTeamNextCapturePoint);

    m_InitialtargetsSet = true;
    return 0;
}
