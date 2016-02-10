#ifndef PickupSpawnTest_h__
#define PickupSpawnTest_h__

#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"
#include "Core/EventBroker.h"
#include "Core/World.h"
#include "Input/InputProxy.h"
#include "Input/KeyboardInputHandler.h"
#include "Input/MouseInputHandler.h"
#include "Core/EKeyDown.h"
#include "Core/EntityFile.h"
#include "Core/SystemPipeline.h"

#include "Core/EntityFilePreprocessor.h"
#include "Core/EntityFileParser.h"
#include "Core/EntityFileWriter.h"

#include "Engine/Collision/ETrigger.h"

//#include "Core/System.h"
//#include "Core/Transform.h"
//#include "Core/ResourceManager.h"
//#include "Core/EntityFileParser.h"
//#include "Core/EPickupSpawned.h"
//#include "Core/EPlayerHealthPickup.h"
#include "Engine/Collision/ETrigger.h"
//#include "Common.h"
//#include <tuple>
#include "Collision/TriggerSystem.h"
#include "Collision/CollisionSystem.h"
#include "Core/EntityFileWriter.h"
#include "Game/Systems/HealthSystem.h"
#include "Game/Systems/PickupSpawnSystem.h"

#include "Core/ResourceManager.h"

class PickupSpawnTest
{
public:
    PickupSpawnTest(int runTestNumber);
    ~PickupSpawnTest();

    void Tick();
    bool TestSucceeded = false;
    int NumLoops = 0;

    bool Game_Loop_OneHundredTimes();

    void TestSetup(int testNumber);
    void DoTouchEvent(EntityID whoDidSomething, EntityID onWhatObject);

private:
    double m_LastTime;
    ConfigFile* m_Config = nullptr;
    EventBroker* m_EventBroker;
    World* m_World;
    SystemPipeline* m_SystemPipeline;
    EntityID m_PlayerID, m_HealthPickupID;
    int m_RunTestNumber;

    EventRelay<PickupSpawnSystem, Events::PlayerHealthPickup> m_HP;
    bool OnHealthPickup(Events::PlayerHealthPickup& e);
    EventRelay<PickupSpawnSystem, Events::PickupSpawned> m_PS;
    bool OnPickupSpawned(Events::PickupSpawned& e);

    bool m_TestStage1Success = false;
    bool m_TestStage2Success = false;


};

#endif
