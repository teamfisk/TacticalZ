#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "PickupSpawnTest.h"

BOOST_AUTO_TEST_SUITE(PickupSpawnTestSuite)

//dont use the same name as the classname in test cases...
BOOST_AUTO_TEST_CASE(PickupSpawnTest_HealthPickupRespawns_PlayerHealthPickupEventTriggers)
{
    PickupSpawnTest game(1);
    bool success = game.Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(PickupSpawnTest_APlayerAtMaxHealth_CantTakeHealthPickup)
{
    PickupSpawnTest game(2);
    bool success = game.Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(PickupSpawnTest_APickupCanRespawnSlowly)
{
    PickupSpawnTest game(3);
    bool success = game.Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_SUITE_END()

PickupSpawnTest::PickupSpawnTest(int runTestNumber)
{
    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    ResourceManager::RegisterType<EntityFile>("EntityFile");

    m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
    LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

    m_EventBroker = new EventBroker();
    m_World = new World();

    // Create system pipeline
    m_SystemPipeline = new SystemPipeline(m_World, m_EventBroker, false, false);
    m_SystemPipeline->AddSystem<HealthSystem>(0);
    m_SystemPipeline->AddSystem<PickupSpawnSystem>(1);

    //must register components (Components.xsd), else you cant create entities. Easiest done by loading a test xsd file
    auto file = ResourceManager::Load<EntityFile>("Schema/Entities/HealthPickup.xml");
    EntityFilePreprocessor fpp(file);
    fpp.RegisterComponents(m_World);
    EntityFileParser fp(file);
    //connect the healthpickup to the world
    m_HealthPickupID = fp.MergeEntities(m_World);

    //create a player
    m_PlayerID = m_World->CreateEntity();
    auto& player = m_World->AttachComponent(m_PlayerID, "Player");

    m_RunTestNumber = runTestNumber;

    //further testsetups
    TestSetup(m_RunTestNumber);

    //init glfw so dt works
    glfwInit();

    //listen to the 2 events that are related to PickupSpawn
    EVENT_SUBSCRIBE_MEMBER(m_HP, &PickupSpawnTest::OnHealthPickup);
    EVENT_SUBSCRIBE_MEMBER(m_PS, &PickupSpawnTest::OnPickupSpawned);
}

bool PickupSpawnTest::OnHealthPickup(Events::PlayerHealthPickup& e) {
    switch (m_RunTestNumber)
    {
    case 1:
        //verify that the event has the correct healthgain number and playerid
        if (e.HealthAmount == 22.0 && e.Player.ID == m_PlayerID) {
            m_TestStage1Success = true;
        }
        break;
    case 2:
        m_TestStage1Success = false;
        break;
    case 3:
        //verify that the event has the correct healthgain number and playerid
        if (e.HealthAmount == 50.0 && e.Player.ID == m_PlayerID) {
            m_TestStage1Success = true;
        }
        break;
    }
    return true;
}
bool PickupSpawnTest::OnPickupSpawned(Events::PickupSpawned& e) {
    switch (m_RunTestNumber)
    {
    case 1:
        //verify that the newly spawned pickup has the same variable values as the original one
        if ((double)e.Pickup["HealthPickup"]["HealthGain"] == 22.0 && (double)e.Pickup["HealthPickup"]["RespawnTimer"] == 2.0) {
            m_TestStage2Success = true;
        }
        break;
    case 2:
        m_TestStage2Success = false;
        break;
    case 3:
        m_TestStage2Success = false;
        break;
    }
    return true;
}

void PickupSpawnTest::TestSetup(int testNumber)
{
    switch (m_RunTestNumber)
    {
    case 1:
    {
        //PickupSpawnTest_HealthPickupRespawns_PlayerHealthPickupEventTriggers
        auto& healthPickupEW = EntityWrapper(m_World, m_HealthPickupID);
        healthPickupEW["HealthPickup"]["RespawnTimer"] = 2.0;
        healthPickupEW["HealthPickup"]["HealthGain"] = 22.0;

        //create a player
        auto& health = m_World->AttachComponent(m_PlayerID, "Health");
        health["Health"] = 20.0;
        health["MaxHealth"] = 100.0;
    }
    break;
    case 2:
    {
        //PickupSpawnTest_APlayerAtMaxHealth_CantTakeHealthPickup
        auto& healthPickupEW = EntityWrapper(m_World, m_HealthPickupID);
        healthPickupEW["HealthPickup"]["RespawnTimer"] = 1.0;
        healthPickupEW["HealthPickup"]["HealthGain"] = 50.0;

        //create a player at max health
        auto& health = m_World->AttachComponent(m_PlayerID, "Health");
        health["Health"] = 100.0;
        health["MaxHealth"] = 100.0;
    }
    break;
    case 3:
    {
        //PickupSpawnTest_APickupCanRespawnSlowly
        auto& healthPickupEW = EntityWrapper(m_World, m_HealthPickupID);
        healthPickupEW["HealthPickup"]["RespawnTimer"] = 100.0;
        healthPickupEW["HealthPickup"]["HealthGain"] = 50.0;

        //create a player
        auto& health = m_World->AttachComponent(m_PlayerID, "Health");
        health["Health"] = 1.0;
        health["MaxHealth"] = 100.0;
    }
    break;
    default:
        break;
    }
    //do the triggerTouch event to get the pickupSpawnTest started
    Events::TriggerTouch eTriggerTouch;
    DoTouchEvent(m_PlayerID, m_HealthPickupID);
}

//generic stuff
void PickupSpawnTest::Tick()
{
    glfwPollEvents();

    //just set dt to 1.0 since we want fast testing
    double dt = 1.0;

    // Iterate through systems and update world!
    m_SystemPipeline->Update(dt);

    m_EventBroker->Swap();
    m_EventBroker->Clear();

    //verify that healthgain event has been published and pickup has respawned
    if (m_RunTestNumber == 1 && m_TestStage1Success && m_TestStage2Success) {
        m_TestSucceeded = true;
    }
    //verify that no healthgain event has been published and that no pickup has respawned
    if (m_NumLoops > 90 && m_RunTestNumber == 2 && !m_TestStage1Success && !m_TestStage2Success) {
        m_TestSucceeded = true;
    }
    //3: verify that the pickup hasnt spawned
    if (m_NumLoops > 90 && m_RunTestNumber == 3 && m_TestStage1Success && !m_TestStage2Success) {
        m_TestSucceeded = true;
    }
}
bool PickupSpawnTest::Game_Loop_OneHundredTimes() {
    //100 loops will be more than enough to do the test
    int loops = 100;
    bool success = false;
    while (loops > 0) {
        Tick();
        m_NumLoops++;
        if (m_TestSucceeded) {
            success = true;
            break;
        }
        loops--;
    }
    return success;
}
PickupSpawnTest::~PickupSpawnTest()
{
    delete m_SystemPipeline;
    delete m_World;
}
void PickupSpawnTest::DoTouchEvent(EntityID whoDidSomething, EntityID onWhatObject) {
    Events::TriggerTouch touchEvent;
    touchEvent.Entity = EntityWrapper(m_World, whoDidSomething);
    touchEvent.Trigger = EntityWrapper(m_World, onWhatObject);
    m_EventBroker->Publish(touchEvent);
}
