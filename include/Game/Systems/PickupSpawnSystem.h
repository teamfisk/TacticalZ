#ifndef PickupSpawnSystem_h__
#define PickupSpawnSystem_h__

#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "Systems/SpawnerSystem.h"
#include "Events/ESpawnerSpawn.h"
#include "Core/EPlayerSpawned.h"
#include "Rendering/ESetCamera.h"
#include "Core/ConfigFile.h"
#include "Core/EPickupSpawned.h"
#include "Core/EPlayerHealthPickup.h";
#include "Engine/Collision/ETrigger.h"

#include "Common.h"
#include <tuple>

class PickupSpawnSystem : public ImpureSystem
{
public:
    PickupSpawnSystem(World* world, EventBroker* eventBroker);

    virtual void Update(double dt) override;

private:
    EventRelay<PickupSpawnSystem, Events::TriggerTouch> m_ETriggerTouch;
    bool OnTriggerTouch(Events::TriggerTouch& e);

    std::vector<std::tuple<glm::vec3,double>> m_ETriggerTouchVector;


};
#endif
