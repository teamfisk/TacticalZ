#include "Systems/EndScreenSystem.h"


EndScreenSystem::EndScreenSystem(SystemParams params)
    : System(params)
    , ImpureSystem()
{
    EVENT_SUBSCRIBE_MEMBER(m_EWin, &EndScreenSystem::OnWin);
}

void EndScreenSystem::Update(double dt)
{

}

bool EndScreenSystem::OnWin(const Events::Win& e)
{
    auto endScreenCameras = m_World->GetComponents("EndScreen");
    if(endScreenCameras == nullptr) {
        LOG_ERROR("Win event recieved but no Endscreen camera is present.");
        return 1;
    }
    for(auto& camera : *endScreenCameras) {
        EntityWrapper entity = EntityWrapper(m_World, camera.EntityID);

        Events::SetCamera event;
        event.CameraEntity = entity;
        m_EventBroker->Publish(event);

        EntityWrapper redSprite = entity.FirstChildByName("SpriteRed");
        EntityWrapper blueSprite = entity.FirstChildByName("SpriteBlue");
        EntityWrapper redText = entity.FirstChildByName("TextRed");
        EntityWrapper blueText = entity.FirstChildByName("TextBlue");

        if (e.TeamThatWon == 2) {
            //Red team won
            if(redSprite.HasComponent("Sprite"))
                (bool&)redSprite["Sprite"]["Visible"] = true;
            if (redText.HasComponent("Text"))
                (bool&)redText["Text"]["Visible"] = true;

            if (blueSprite.HasComponent("Sprite"))
                (bool&)blueSprite["Sprite"]["Visible"] = false;
            if (blueText.HasComponent("Text"))
                (bool&)blueText["Text"]["Visible"] = false;

        }else if (e.TeamThatWon == 3) {
            //Blue team won
            if (redSprite.HasComponent("Sprite"))
                (bool&)redSprite["Sprite"]["Visible"] = false;
            if (redText.HasComponent("Text"))
                (bool&)redText["Text"]["Visible"] = false;

            if (blueSprite.HasComponent("Sprite"))
                (bool&)blueSprite["Sprite"]["Visible"] = true;
            if (blueText.HasComponent("Text"))
                (bool&)blueText["Text"]["Visible"] = true;
        } else {
            //No team won?
            if (redSprite.HasComponent("Sprite"))
                (bool&)redSprite["Sprite"]["Visible"] = false;
            if (blueSprite.HasComponent("Sprite"))
                (bool&)blueSprite["Sprite"]["Visible"] = false;

            if (redText.HasComponent("Text"))
                (bool&)redText["Text"]["Visible"] = false;
            if (blueText.HasComponent("Text"))
                (bool&)blueText["Text"]["Visible"] = false;
        }
    }


    return 1;
}
