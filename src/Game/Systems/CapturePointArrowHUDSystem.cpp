#include "Systems/CapturePointArrowHUDSystem.h"

CapturePointArrowHUDSystem::CapturePointArrowHUDSystem(SystemParams params)
    : System(params)
    , ImpureSystem()
{
    EVENT_SUBSCRIBE_MEMBER(m_ECapturedEvent, &CapturePointArrowHUDSystem::OnCapturePointCaptured);
}


void CapturePointArrowHUDSystem::Update(double dt)
{
    bool LoadCheck = true;
    int redTeam;
    int blueTeam;
    int spectatorTeam;

    //Get list for all CapturePointArrowHUDComponents
    auto ArrowHUDs = m_World->GetComponents("CapturePointArrowHUD");
    auto CapturePoints = m_World->GetComponents("CapturePoint");
    if(ArrowHUDs == nullptr) {
        return;
    }

    for(auto& cArrowHUD : *ArrowHUDs) {
        //Get what team the current arrow corresponds to
        EntityWrapper ArrowEntity = EntityWrapper(m_World, cArrowHUD.EntityID);
        if (!ArrowEntity.Valid()) {
            continue;
        }
        if(!ArrowEntity.HasComponent("Team")) {
            continue;
        }
        auto cTeam = ArrowEntity["Team"];
        int currentTeam = (int)cTeam["Team"];

        if (LoadCheck) {
            redTeam = (int)cTeam["Team"].Enum("Red");
            blueTeam = (int)cTeam["Team"].Enum("Blue");
            spectatorTeam = (int)cTeam["Team"].Enum("Spectator");
            LoadCheck = false;


            if (!m_InitialtargetsSet) {
                glm::vec3 target1, target2;
                EntityWrapper home1, home2;

                for (auto& cCP : *CapturePoints) {
                    auto homePointTeam = (int)cCP["HomePointForTeam"];
                    auto CPID = (int)cCP["CapturePointNumber"];

                    if(CPID == 0) {
                        //Home point for one team
                        home1 = EntityWrapper(m_World, cCP.EntityID);
                    } else if (CPID == 1) {
                        //First target for one team, so save it for later use.
                        target1 = Transform::AbsolutePosition(EntityWrapper(m_World, cCP.EntityID));
                    } else if (CPID == 3) {
                        //First target for one team, so save it for later use.
                        target2 = Transform::AbsolutePosition(EntityWrapper(m_World, cCP.EntityID));
                    } else if (CPID == 4) {
                        //Home point for one team
                        home2 = EntityWrapper(m_World, cCP.EntityID);
                    }
                }
                //Check what team is the owner of Home1 and set their target to the next capturepoint
                if((int)home1["CapturePoint"]["HomePointForTeam"] == redTeam) {
                    m_RedTeamCurrentTarget = target1;
                } else if ((int)home1["CapturePoint"]["HomePointForTeam"] == blueTeam) {
                    m_BlueTeamCurrentTarget = target1;
                }

                //Check what team is the owner of Home2 and set their target to the next capturepoint
                if ((int)home2["CapturePoint"]["HomePointForTeam"] == redTeam) {
                    m_RedTeamCurrentTarget = target2;
                } else if ((int)home2["CapturePoint"]["HomePointForTeam"] == blueTeam) {
                    m_BlueTeamCurrentTarget = target2;
                }
            }


        }

        //if red team, get red team next point, otherwise blue team next point.
        //Untill this is awailable we will just use the hardcoded value in the component.
        //This will also give us a position, so we wont need to loop through all capturePoints.
        glm::vec3 pos;
        if(currentTeam == redTeam) {
            pos = m_RedTeamCurrentTarget;
        } else if (currentTeam == blueTeam) {
            pos = m_BlueTeamCurrentTarget;
        }

        pos = currentTeam == redTeam ? m_RedTeamCurrentTarget : currentTeam == blueTeam ? m_BlueTeamCurrentTarget : glm::vec3(0.f);

        glm::vec3& arrowOri = ArrowEntity["Transform"]["Orientation"];
        glm::vec3 lookVector = glm::normalize(Transform::AbsolutePosition(ArrowEntity) - pos); //Maybe should be player instead
        float pitch = std::asin(-lookVector.y);
        float yaw = std::atan2(lookVector.x, lookVector.z);
        arrowOri.x = pitch;
        arrowOri.y = yaw;
        arrowOri.z = 0.f;
        EntityWrapper parent = ArrowEntity.Parent();
        if (parent.Valid()) {
            arrowOri -= Transform::AbsoluteOrientationEuler(parent);
        }
    }
}

bool CapturePointArrowHUDSystem::OnCapturePointCaptured(Events::Captured& e)
{
    if (!e.NextCapturePoint.HasComponent("Team"))
    {
        return 0;
    }

    auto cTeam = e.NextCapturePoint["Team"];
    
    int redTeam = (int)cTeam["Team"].Enum("Red");
    int blueTeam = (int)cTeam["Team"].Enum("Blue");
    int spectatorTeam = (int)cTeam["Team"].Enum("Spectator");
    int target = -1;

    if (e.NextCapturePoint.HasComponent("CapturePoint")) {
        target = (int)e.NextCapturePoint["CapturePoint"]["CapturePointNumber"];
    } else {
        return 0;
    }

    if(e.TeamNumberThatCapturedCapturePoint == redTeam) {
        m_RedTeamCurrentTarget = Transform::AbsolutePosition(e.NextCapturePoint);
    } else if (e.TeamNumberThatCapturedCapturePoint == blueTeam) {
        m_BlueTeamCurrentTarget = Transform::AbsolutePosition(e.NextCapturePoint);;
    }
}
