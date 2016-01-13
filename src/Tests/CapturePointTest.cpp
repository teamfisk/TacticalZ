#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "CapturePointTest.h"
#include "Game/HealthSystem.h"

#include "Collision/TriggerSystem.h"
#include "Collision/CollisionSystem.h"
#include "Core/EntityFileWriter.h"
#include "Game/CapturePointSystem.h"

BOOST_AUTO_TEST_SUITE(CapturePointTestSuite)

//dont use the same name as the classname in test cases...
BOOST_AUTO_TEST_CASE(CapturePointTest1_OnePlayerOnCapturePoint)
{
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
BOOST_AUTO_TEST_CASE(CapturePointTest2_TwoPlayersOnCapturePoint)
{
    CapturePointTest game(2);
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
BOOST_AUTO_TEST_CASE(CapturePointTest3_NoPlayersOnCapturePoint)
{
    CapturePointTest game(3);
    //100 loops will be more than enough to do the test
    int loops = 100;
    bool success = false;
    while (loops > 0) {
        game.Tick();
        //successCheck needs to know when were close to 100 to check if anything happened then (NumLoops)
        game.NumLoops++;
        if (game.TestSucceeded) {
            success = true;
            break;
        }
        loops--;
    }
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest4_TwoCapturePointsBeingCaptured)
{
    CapturePointTest game(4);
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
BOOST_AUTO_TEST_CASE(CapturePointTest5_SameCapturePointContestedAndTakenOver)
{
    CapturePointTest game(5);
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
BOOST_AUTO_TEST_CASE(CapturePointTest6_Team1CapturedTheLastPointAndWon)
{
    CapturePointTest game(6);
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

    //create 2 players and 3 capturepoints for testing
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
    ComponentWrapper& capturePoint = m_World->AttachComponent(capturePointID, "CapturePoint");
    //this capturePoint is homeBase for team 2
    capturePoint["IsHomeCapturePointForTeamNumber"] = 2;
    capturePoint["CapturePointNumber"] = 0;
    m_CapturePointID = capturePointID;

    EntityID capturePointID2 = m_World->CreateEntity();
    ComponentWrapper& capturePoint2 = m_World->AttachComponent(capturePointID2, "CapturePoint");
    //this capturePoint is homeBase for team 1
    capturePoint2["IsHomeCapturePointForTeamNumber"] = 0;
    capturePoint2["CapturePointNumber"] = 1;
    m_CapturePointID2 = capturePointID2;

    EntityID capturePointID3 = m_World->CreateEntity();
    ComponentWrapper& capturePoint3 = m_World->AttachComponent(capturePointID3, "CapturePoint");
    //this capturePoint is homeBase for team 1
    capturePoint3["IsHomeCapturePointForTeamNumber"] = 1;
    capturePoint3["CapturePointNumber"] = 2;
    m_CapturePointID3 = capturePointID3;

    m_RunTestNumber = runTestNumber;
    //add some touch/leave events
    switch (runTestNumber)
    {
    case 1:
        TestSetup1_OnePlayerOnCapturePoint();
        break;
    case 2:
        TestSetup2_TwoPlayersOnCapturePoint();
        break;
    case 3:
        TestSetup3_NoPlayersOnCapturePoint();
        break;
    case 4:
        TestSetup4_TwoCapturePointsBeingCaptured();
        break;
    case 5:
        TestSetup5_SameCapturePointContestedAndTakenOver();
        break;
    case 6:
        TestSetup6_Team1CapturedTheLastPointAndWon();
        break;
    default:
        break;
    }

    //init glfw so dt works
    glfwInit();
}

CapturePointTest::~CapturePointTest()
{
    delete m_SystemPipeline;
    delete m_World;
    delete m_EventBroker;
}

void CapturePointTest::TestSetup1_OnePlayerOnCapturePoint()
{
    Events::TriggerTouch touchEvent;
    Events::TriggerLeave leaveEvent;

    //player touches,leaves,touches m_CapturePointID. and enters m_CapturePointID3
    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(touchEvent);

    leaveEvent.Entity = m_PlayerID;
    leaveEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(leaveEvent);

    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(touchEvent);

    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID3;
    m_EventBroker->Publish(touchEvent);
}
void CapturePointTest::TestSetup2_TwoPlayersOnCapturePoint()
{
    Events::TriggerTouch touchEvent;
    Events::TriggerLeave leaveEvent;

    //player touches,leaves m_CapturePointID. and enters m_CapturePointID3
    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(touchEvent);

    leaveEvent.Entity = m_PlayerID;
    leaveEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(leaveEvent);

    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID3;
    m_EventBroker->Publish(touchEvent);

    //player2 touches m_CapturePointID,m_CapturePointID2
    touchEvent.Entity = m_PlayerID2;
    touchEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(touchEvent);

    touchEvent.Entity = m_PlayerID2;
    touchEvent.Trigger = m_CapturePointID2;
    m_EventBroker->Publish(touchEvent);
}
void CapturePointTest::TestSetup3_NoPlayersOnCapturePoint()
{
    Events::TriggerTouch touchEvent;
    Events::TriggerLeave leaveEvent;

    //player1 touches and leaves m_CapturePointID
    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(touchEvent);

    leaveEvent.Entity = m_PlayerID;
    leaveEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(leaveEvent);

    //player2 touches and leaves m_CapturePointID2
    touchEvent.Entity = m_PlayerID2;
    touchEvent.Trigger = m_CapturePointID2;
    m_EventBroker->Publish(touchEvent);

    leaveEvent.Entity = m_PlayerID2;
    leaveEvent.Trigger = m_CapturePointID2;
    m_EventBroker->Publish(leaveEvent);

}
void CapturePointTest::TestSetup4_TwoCapturePointsBeingCaptured()
{
    Events::TriggerTouch touchEvent;

    //player1 touches m_CapturePointID3
    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID3;
    m_EventBroker->Publish(touchEvent);

    //player2 touches m_CapturePointID
    touchEvent.Entity = m_PlayerID2;
    touchEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(touchEvent);
}
void CapturePointTest::TestSetup5_SameCapturePointContestedAndTakenOver()
{
    //NOTE: setup events need to trigger first then the real event will be allowed by the system later
    Events::TriggerTouch touchEvent;

    //"SETUP" homebase->same capturep
    //player1 touches m_CapturePointID3
    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID3;
    m_EventBroker->Publish(touchEvent);

    //player2 touches m_CapturePointID
    touchEvent.Entity = m_PlayerID2;
    touchEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(touchEvent);

    //contested same, player1 touches the contested
    //player1 touches m_CapturePointID2
    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID2;
    m_EventBroker->Publish(touchEvent);

    //player2 does nothing

}
void CapturePointTest::TestSetup6_Team1CapturedTheLastPointAndWon()
{
    //NOTE: setup events need to trigger first then the real event will be allowed by the system later
    Events::TriggerTouch touchEvent;

    //"SETUP" team1 captures point 2,3
    //player1 touches m_CapturePointID3
    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID3;
    m_EventBroker->Publish(touchEvent);

    //player1 touches m_CapturePointID2
    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID2;
    m_EventBroker->Publish(touchEvent);

    //team1 captures point 1
    //player1 touches m_CapturePointID
    touchEvent.Entity = m_PlayerID;
    touchEvent.Trigger = m_CapturePointID;
    m_EventBroker->Publish(touchEvent);

    //player2 does nothing
}
void CapturePointTest::TestSuccess1() {
    //TestSetup1_OnePlayerOnCapturePoint

    //if ammocount reaches -- we know the test has succeeded, i.e. a shot has been fired
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "CapturePoint")["OwnedBy"];
    if (ownedByID3 == 1)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess2() {
    //TestSetup2_TwoPlayersOnCapturePoint
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "CapturePoint")["OwnedBy"];
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "CapturePoint")["OwnedBy"];
    if (ownedByID3 == 1 && ownedByID1 == 2)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess3() {
    //TestSetup3_NoPlayersOnCapturePoint
    //only do this test if were at the final loopcount
    //if any capturePoint changed then, its a failure else a success
    if (NumLoops == 95) {
        TestSucceeded = true;
        int ownedByID1 = m_World->GetComponent(m_CapturePointID, "CapturePoint")["OwnedBy"];
        int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "CapturePoint")["OwnedBy"];
        int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "CapturePoint")["OwnedBy"];
        if (ownedByID1 != 0 || ownedByID2 != 0 || ownedByID3 != 0)
            TestSucceeded = false;
    }
}
void CapturePointTest::TestSuccess4() {
    //TestSetup4_TwoCapturePointsBeingCaptured
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "CapturePoint")["OwnedBy"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "CapturePoint")["OwnedBy"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "CapturePoint")["OwnedBy"];
    if (ownedByID1 == 2 && ownedByID3 == 1)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess5() {
    //TestSetup5_SameCapturePointContestedAndTakenOver
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "CapturePoint")["OwnedBy"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "CapturePoint")["OwnedBy"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "CapturePoint")["OwnedBy"];
    if (ownedByID1 == 2 && ownedByID2 == 1 && ownedByID3 == 1)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess6() {
    //NOTE: the actual win-event will have to be manually checked if it triggered or not
    //TestSetup6_Team1CapturedTheLastPointAndWon
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "CapturePoint")["OwnedBy"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "CapturePoint")["OwnedBy"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "CapturePoint")["OwnedBy"];
    if (ownedByID1 == 1 && ownedByID2 == 1 && ownedByID3 == 1)
        TestSucceeded = true;
}
void CapturePointTest::Tick()
{
    glfwPollEvents();

    //double currentTime = glfwGetTime();
    //double dt = currentTime - m_LastTime;
    //m_LastTime = currentTime;

    //just set dt to 10.0 since we want fast testing
    double dt = 10.0;

    // Iterate through systems and update world!
    m_SystemPipeline->Update(m_World, dt);

    m_EventBroker->Swap();
    m_EventBroker->Clear();

    switch (m_RunTestNumber)
    {
    case 1:
        TestSuccess1();
        break;
    case 2:
        TestSuccess2();
        break;
    case 3:
        TestSuccess3();
        break;
    case 4:
        TestSuccess4();
        break;
    case 5:
        TestSuccess5();
        break;
    case 6:
        TestSuccess6();
        break;
    default:
        break;
    }
}
