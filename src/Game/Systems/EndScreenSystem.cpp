#include "Systems/EndScreenSystem.h"


EndScreenSystem::EndScreenSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EWin, &EndScreenSystem::OnWin);
}

void EndScreenSystem::Update(double dt)
{

}

bool EndScreenSystem::OnWin(const Events::Win&)
{
    
}
