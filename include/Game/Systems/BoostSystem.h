#ifndef BoostSystem_h__
#define BoostSystem_h__

#include "Core/System.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFile.h"
#include "Core/EPlayerDamage.h"
#include "Common.h"

class BoostSystem : public System
{
public:
    BoostSystem(SystemParams params);

private:
    EventRelay<BoostSystem, Events::PlayerDamage> m_EPlayerDamage;
    bool OnPlayerDamage(Events::PlayerDamage& e);

    std::string DetermineClass(EntityWrapper player);
};
#endif