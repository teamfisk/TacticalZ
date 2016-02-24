#ifndef Systems_BackgroundMusicSystem_h__
#define Systems_BackgroundMusicSystem_h__

#include "../Engine/Core/System.h"
#include "../Engine/Core/EventBroker.h"

// Handles transitions between BGM's depending on the current state of the game.
class BackgroundMusicSystem : public PureSystem
{
public:
    BackgroundMusicSystem(SystemParams params);
    ~BackgroundMusicSystem();
    void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cEmitter, double dt) override;

private:
    void mainMenuLoop();

    const std::string menu = "Audio/crosscounter.wav";
};

#endif // ifndef 
