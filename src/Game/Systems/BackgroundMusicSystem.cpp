#include "../Game/Systems/BackgroundMusicSystem.h"

BackgroundMusicSystem::BackgroundMusicSystem(SystemParams params)
    : System(params)
    , PureSystem("SoundEmitter")
{

}

BackgroundMusicSystem::~BackgroundMusicSystem()
{
    
}

void BackgroundMusicSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cEmitter, double dt)
{

}

void BackgroundMusicSystem::mainMenuLoop()
{

}
