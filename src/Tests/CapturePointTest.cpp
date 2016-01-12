#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "CapturePointTest.h"
#include "Game/HealthSystem.h"

#include "Collision/TriggerSystem.h"
#include "Collision/CollisionSystem.h"
#include "Core/EntityFileWriter.h"
#include "Game/CapturePointSystem.h"

BOOST_AUTO_TEST_SUITE(ShootEventTestSuite)

//dont use the same name as the classname in test cases...
BOOST_AUTO_TEST_CASE(CapturePointTest1)
{
    //Test firing primary weapon
    CapturePointTest game(1);
    //100 loops will be more than enough to do the test
    int loops = 100;
    bool success = false;
    while (loops > 0) {
        game.Tick();
        if (game.TestSucceeded) {
            success = true;
            break;
        }
        loops--;
    }
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_SUITE_END()

CapturePointTest::CapturePointTest(int runTestNumber)
{
    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    ResourceManager::RegisterType<EntityFile>("EntityFile");

    m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
    std::string mapToLoad = m_Config->Get<std::string>("Debug.LoadMap", "");
    LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

    // Create the core event broker
    m_EventBroker = new EventBroker();

    // Create a world
    m_World = new World();

    // Create system pipeline
    m_SystemPipeline = new SystemPipeline(m_EventBroker);
    m_SystemPipeline->AddSystem<PlayerSystem>(0);
    m_SystemPipeline->AddSystem<HealthSystem>(0);
    m_SystemPipeline->AddSystem<CollisionSystem>(1);
    m_SystemPipeline->AddSystem<TriggerSystem>(1);
    m_SystemPipeline->AddSystem<CapturePointSystem>(1);

    if (!mapToLoad.empty()) {
        auto file = ResourceManager::Load<EntityFile>(mapToLoad);
        EntityFilePreprocessor fpp(file);
        fpp.RegisterComponents(m_World);
        EntityFileParser fp(file);
        fp.MergeEntities(m_World);
    }

    //The Test
    //create entity which has transform,player,model,health in it. i.e. is a player
    EntityID playerID = m_World->CreateEntity();
    m_PlayerID = playerID;
    ComponentWrapper& player = m_World->AttachComponent(playerID, "Player");
    ComponentWrapper health = m_World->AttachComponent(playerID, "Health");
    player["TeamNumber"] = 1;

    EntityID playerID2 = m_World->CreateEntity();
    ComponentWrapper& player2 = m_World->AttachComponent(playerID2, "Player");
    m_PlayerID2 = playerID2;
    ComponentWrapper health2 = m_World->AttachComponent(playerID2, "Health");
    player2["TeamNumber"] = 2;

    EntityID capturePointID = m_World->CreateEntity();
    ComponentWrapper& capPointComp = m_World->AttachComponent(capturePointID, "CapturePoint");
    m_CapturePointID = capturePointID;
    m_RunTestNumber = runTestNumber;

    //add some touch/leave events
    Events::TriggerTouch eTriggerTouched;
    eTriggerTouched.Entity = m_PlayerID;
    eTriggerTouched.Trigger = m_CapturePointID;
    m_EventBroker->Publish(eTriggerTouched);

    Events::TriggerTouch eTriggerTouched2;
    eTriggerTouched2.Entity = m_PlayerID2;
    eTriggerTouched2.Trigger = m_CapturePointID;
    m_EventBroker->Publish(eTriggerTouched2);

    Events::TriggerLeave eTriggerLeft;
    eTriggerLeft.Entity = m_PlayerID;
    eTriggerLeft.Trigger = m_CapturePointID;
    m_EventBroker->Publish(eTriggerLeft);

    Events::TriggerTouch eTriggerTouched3;
    eTriggerTouched3.Entity = m_PlayerID;
    eTriggerTouched3.Trigger = m_CapturePointID;
    m_EventBroker->Publish(eTriggerTouched3);

}

CapturePointTest::~CapturePointTest()
{
    delete m_SystemPipeline;
    delete m_World;
    delete m_EventBroker;
}

void CapturePointTest::Tick()
{
    glfwPollEvents();

    double currentTime = glfwGetTime();
    double dt = currentTime - m_LastTime;
    m_LastTime = currentTime;

    // Iterate through systems and update world!
    m_SystemPipeline->Update(m_World, dt);

    m_EventBroker->Swap();
    m_EventBroker->Clear();

}
