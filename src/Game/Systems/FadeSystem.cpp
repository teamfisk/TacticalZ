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

    if (!(bool)cFade["Out"]) {
        dTime *= -1;
    }

    double fadeTime = cFade["FadeTime"];
    double& currentTime = cFade["Time"];
    currentTime += dTime;

    if(currentTime > fadeTime) {
        currentTime = 0.0;
    }
    if(currentTime < 0.0) {
        currentTime = fadeTime;
    }
    double ratio = currentTime/fadeTime;

    if(entity.HasComponent("Model")){
        ((glm::vec4&)entity["Model"]["Color"]).a = ratio;
    }
    if (entity.HasComponent("Sprite")) {
        ((glm::vec4&)entity["Sprite"]["Color"]).a = ratio;

    }
}