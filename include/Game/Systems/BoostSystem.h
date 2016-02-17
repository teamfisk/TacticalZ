#ifndef BoostSystem_h__
#define BoostSystem_h__

#include "Core/System.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFileParser.h"
#include "Core/EPlayerDamage.h"
#include "Common.h"
#include <tuple>

#include "Rendering/Util/CommonFunctions.h"

class BoostSystem : public System
{
public:
    BoostSystem(SystemParams params);

private:
    EventRelay<BoostSystem, Events::PlayerDamage> m_EPlayerDamage;
    bool OnPlayerDamage(Events::PlayerDamage& e);

    std::string determineClass(EntityWrapper player);
};
#endif