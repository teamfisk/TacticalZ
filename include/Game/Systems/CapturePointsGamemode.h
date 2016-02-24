#include "Core/System.h"
#include "Events/EGameStart.h"
#include "Events/EGameEnd.h"
#include "Events/ECaptured.h"
#include "Events/EWin.h"

class CapturePointsGamemode : public ImpureSystem
{
public:
    CapturePointsGamemode(SystemParams params);

    virtual void Update(double dt) override;

private:
    bool m_GameRuning = false;
    double m_GameTime = 0.0;

    EventRelay<CapturePointsGamemode, Events::GameStart> m_EGameStart;
    bool OnGameStart(const Events::GameStart& e);
    EventRelay<CapturePointsGamemode, Events::Captured> m_ECaptured;
    bool OnCaptured(const Events::Captured& e);

    boost::optional<ComponentWrapper> currentGamemode();
    bool isCurrentGamemode();
    boost::optional<ComponentInfo::EnumType> winCondition();
    void setRunning(bool running);
};