#include "Systems/CapturePointHUDSystem.h"

CapturePointHUDSystem::CapturePointHUDSystem(SystemParams params)
    : System(params)
    , ImpureSystem()
{
}


void CapturePointHUDSystem::Update(double dt)
{
    bool LoadCheck = true;
    int redTeam;
    int blueTeam;
    int spectatorTeam;

    auto CapturePointHUDElements = m_World->GetComponents("CapturePointHUD");
    auto CapturePoints = m_World->GetComponents("CapturePoint");
    if (CapturePointHUDElements == nullptr) {
        return;
    }

    if(!CapturePointHUDElements) {
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
}