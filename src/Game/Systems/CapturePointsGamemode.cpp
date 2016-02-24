#include "Systems/CapturePointsGamemode.h"

CapturePointsGamemode::CapturePointsGamemode(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_ECaptured, &CapturePointsGamemode::OnCaptured);
    EVENT_SUBSCRIBE_MEMBER(m_EGameStart, &CapturePointsGamemode::OnGameStart);
}

void CapturePointsGamemode::Update(double dt)
{
    if (!m_GameRuning) {
        return;
    }

    auto cGamemode = currentGamemode();
    if (!cGamemode) {
        setRunning(false);
        return;
    }

    m_GameTime += dt;
    (double&)(*cGamemode)["RoundTime"] = m_GameTime;
}

bool CapturePointsGamemode::OnGameStart(const Events::GameStart& e)
{
    if (!isCurrentGamemode()) {
        return false;
    }

    setRunning(true);

    return true;
}

bool CapturePointsGamemode::OnCaptured(const Events::Captured& e)
{
    if (!m_GameRuning) {
        return false;
    }

    auto cGamemode = currentGamemode();
    if (!cGamemode) {
        return false;
    }

    auto winningTeam = winCondition();
    if (winningTeam) {
        setRunning(false);

        Events::Win e;
        e.Team = *winningTeam;
        m_EventBroker->Publish(e);
    }

    return true;
}

boost::optional<ComponentWrapper> CapturePointsGamemode::currentGamemode()
{
    auto cGamemodes = m_World->GetComponents("Gamemode");
    if (cGamemodes->begin() == cGamemodes->end()) {
        return boost::none;
    }
    return *cGamemodes->begin();
}

bool CapturePointsGamemode::isCurrentGamemode()
{
    auto cGamemode = currentGamemode();
    if (!cGamemode) {
        return false;
    }

    // Check that we're the current gamemode
    if ((ComponentInfo::EnumType)(*cGamemode)["Gamemode"] == (*cGamemode)["Gamemode"].Enum("CapturePoints")) {
        return true;
    } else {
        return false;
    }
}

boost::optional<ComponentInfo::EnumType> CapturePointsGamemode::winCondition()
{
    auto cCapturePoints = m_World->GetComponents("CapturePoint");
    
    // If all capture points are owned by the same team, they won
    ComponentInfo::EnumType winningTeam = -1;
    for (auto& c : *cCapturePoints) {
        EntityWrapper e(m_World, c.EntityID);
        ComponentInfo::EnumType team = e["Team"]["Team"];
        if (winningTeam != -1 && winningTeam != team) {
            return boost::none;
        } else {
            winningTeam = team;
        }
    }

    return winningTeam;
}

void CapturePointsGamemode::setRunning(bool running)
{
    m_GameRuning = running;
    if (running) {
        m_GameTime = 0.0;
    }

    auto cGamemode = currentGamemode();
    if (cGamemode) {
        m_GameRuning = running;
        if (running) {
            (*cGamemode)["RoundTime"] = 0.0;
        }
    }
}
