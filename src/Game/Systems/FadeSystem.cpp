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
    Field<double> currentTime = cFade["Time"];
    currentTime += dTime;

    if(currentTime > fadeTime) {
        currentTime = 0.0 + fadeTime * (bool)cFade["Loop"];
        if ((bool)cFade["Reverse"]) {
            (Field<bool>)cFade["Out"] = !(bool)cFade["Out"];
            currentTime = fadeTime;
        }
    }
    if(currentTime < 0.0) {
        currentTime = fadeTime * (bool)cFade["Loop"];
        if ((bool)cFade["Reverse"]) {
            (Field<bool>)cFade["Out"] = !(bool)cFade["Out"];
            currentTime = 0.0;
        }
    }
    double ratio = currentTime/fadeTime;

    if(entity.HasComponent("Model")){
        ((Field<glm::vec4>)entity["Model"]["Color"]).w(ratio);
    }
    if (entity.HasComponent("Sprite")) {
        ((Field<glm::vec4>)entity["Sprite"]["Color"]).w(ratio);

    }
}