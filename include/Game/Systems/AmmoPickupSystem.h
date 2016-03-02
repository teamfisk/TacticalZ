#ifndef AmmoPickupSystem_h__
#define AmmoPickupSystem_h__

#include "Core/System.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFileParser.h"
#include "Core/EPickupSpawned.h"
#include "Core/EAmmoPickup.h"
#include "Engine/Collision/ETrigger.h"
#include "Common.h"

class AmmoPickupSystem : public ImpureSystem
{
public:
    AmmoPickupSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    EventRelay<AmmoPickupSystem, Events::TriggerTouch> m_ETriggerTouch;
    bool OnTriggerTouch(Events::TriggerTouch& e);

    struct NewAmmoPickup {
        glm::vec3 Pos;
        double AmmoGain;
        double RespawnTimer;
        double DecreaseThisRespawnTimer;
    };
    std::vector<NewAmmoPickup> m_ETriggerTouchVector;
};
#endif
