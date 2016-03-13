#include "Systems/FadeSystem.h"

void FadeSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cFade, double dt)
{
    double dTime = dt;
    if (dTime == 0.0) {
        return;
    }

    if (!entity.Valid()) {
        return;
    }

    if ((bool)cFade["Out"]) {
        dTime *= -1;
    }

    if((bool)cFade["Reverse"]) {
        dTime *= 2;
    }

    double fadeTime = cFade["FadeTime"];
    double& currentTime = cFade["Time"];
    currentTime += dTime;

    if(currentTime > fadeTime) {
        currentTime = 0.0;
        if ((bool)cFade["Reverse"]) {
            (bool&)cFade["Out"] = !(bool)cFade["Out"];
            currentTime = fadeTime;
        }
    }
    if(currentTime < 0.0) {
        currentTime = fadeTime;
        if ((bool)cFade["Reverse"]) {
            (bool&)cFade["Out"] = !(bool)cFade["Out"];
            currentTime = 0.0;
        }
    }
    double ratio = currentTime/fadeTime;

    if(entity.HasComponent("Model")){
        ((glm::vec4&)entity["Model"]["Color"]).a = ratio;
    }
    if (entity.HasComponent("Sprite")) {
        ((glm::vec4&)entity["Sprite"]["Color"]).a = ratio;

    }
}