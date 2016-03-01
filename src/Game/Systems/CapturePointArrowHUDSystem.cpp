#include "Systems/CapturePointArrowHUDSystem.h"

CapturePointArrowHUDSystem::CapturePointArrowHUDSystem(SystemParams params)
    : System(params)
    , ImpureSystem()
{
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
        EntityWrapper teamEntity = ArrowEntity.FirstParentWithComponent("Team");
        if (!teamEntity.Valid()) {
            continue;
        }
        auto cTeam = teamEntity["Team"];
        int currentTeam = (int)cTeam["Team"];

        if (LoadCheck) {
            redTeam = (int)cTeam["Team"].Enum("Red");
            blueTeam = (int)cTeam["Team"].Enum("Blue");
            spectatorTeam = (int)cTeam["Team"].Enum("Spectator");
            LoadCheck = false;
        }

        //if red team, get red team next point, otherwise blue team next point.
        //Untill this is awailable we will just use the hardcoded value in the component.
        //This will also give us a position, so we wont need to loop through all capturePoints.
        glm::vec3 pos;
        int target = cArrowHUD["CurrentTarget"];
        for(auto& cCP : *CapturePoints) {
            if((int)cCP["CapturePointNumber"] == target) {
                EntityWrapper CPEntity = EntityWrapper(m_World, cCP.EntityID);
                pos = Transform::AbsolutePosition(CPEntity);
                break;
            }
        }
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

    /*
    bool LoadCheck = true;
    int redTeam;
    int blueTeam;
    int spectatorTeam;

    auto CapturePointHUDElements = m_World->GetComponents("CapturePointHUD");
    auto CapturePoints = m_World->GetComponents("CapturePoint");
    if (CapturePointHUDElements == nullptr) {
        return;
    }

    if (!CapturePointHUDElements) {
        return;
    }

    for (auto& cCapturePointHUD : *CapturePointHUDElements) {
        int HUD_ID = cCapturePointHUD["CapturePointNumber"];
        EntityWrapper entityHUD = EntityWrapper(m_World, cCapturePointHUD.EntityID);
        EntityWrapper entityHUDparent = entityHUD.Parent();

        for (auto& cCapturePoint : *CapturePoints) {
            EntityWrapper entityCP = EntityWrapper(m_World, cCapturePoint.EntityID);

            //Check if the HUD corresponds to the Capture Point Number
            if (HUD_ID == (int)entityCP["CapturePoint"]["CapturePointNumber"]) {
                ComponentWrapper& teamComponent = entityCP["Team"];
                if (LoadCheck) {
                    redTeam = (int)teamComponent["Team"].Enum("Red");
                    blueTeam = (int)teamComponent["Team"].Enum("Blue");
                    spectatorTeam = (int)teamComponent["Team"].Enum("Spectator");
                    LoadCheck = false;
                }
                //Color hud with team color
                auto capturePointTeam = (int)teamComponent["Team"];
                entityHUDparent["Sprite"]["Color"] = capturePointTeam == blueTeam ? glm::vec4(0, 0.2f, 1, 0.7f) : capturePointTeam == redTeam ? glm::vec4(1, 0.0f, 0, 0.7f) : glm::vec4(1, 1, 1, 0.3f);

                //Progress is scaled with time
                double currentCaptureTime = (double)entityCP["CapturePoint"]["CaptureTimer"];
                double progress = glm::abs(currentCaptureTime)/15.0;
                int currentCapturingTeam = currentCaptureTime > 0 ? redTeam : currentCaptureTime < 0 ? blueTeam : spectatorTeam;
                ((glm::vec3&)entityHUD["Transform"]["Orientation"]).z = currentCapturingTeam == redTeam ? glm::half_pi<float>()+glm::pi<float>() : glm::half_pi<float>();
                glm::vec4 fillColor = currentCapturingTeam == redTeam ? glm::vec4(1, 0.f, 0, 0.7f) : glm::vec4(0, 0.2f, 1, 0.7f);
                entityHUD["Fill"]["Color"] = fillColor;
                entityHUD["Fill"]["Percentage"] = progress;
            }
        }
    }
    */
}