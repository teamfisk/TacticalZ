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
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest2_TwoPlayersOnCapturePoint)
{
    CapturePointTest game(2);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest3_NoPlayersOnCapturePoint)
{
    CapturePointTest game(3);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest4_TwoCapturePointsBeingCaptured)
{
    CapturePointTest game(4);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest5_SameCapturePointContestedAndTakenOver)
{
    CapturePointTest game(5);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest6_Team1CapturedTheLastPointAndWon)
{
    CapturePointTest game(6);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest7_Team1ForcesTeam2sNextCapturePointToGoBackwards1Step)
{
    CapturePointTest game(7);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest8_Team2ForcesTeam1sNextCapturePointToGoForwards1Step)
{
    CapturePointTest game(8);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    //The system will process the events, hence it will take a while before we can read anything
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_SUITE_END()

bool CapturePointTest::CapturePoint_Game_Loop_OneHundredTimes() {
    //CapturePointTest game(testNumber);
    //100 loops will be more than enough to do the test
    int loops = 100;
    bool success = false;
    while (loops > 0) {
        Tick();
        NumLoops++;
        if (TestSucceeded) {
            success = true;
            break;
        }
        loops--;
    }
    return success;
}

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

    /*
    ---TESTSETUP---
    default: 2 players
    healthcomponent
    3 capturepoints
    capturepoint(1) = home for team number 2
    capturepoint3 = home for team number 1
    */
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

    //further testsetups:i.e. add some initial touch/leave events
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
    case 7:
        //default homecapturepoints
        TestSetup7();
        break;
    case 8:
        //switch sides
        capturePoint["IsHomeCapturePointForTeamNumber"] = 1;
        capturePoint3["IsHomeCapturePointForTeamNumber"] = 2;
        TestSetup8();
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
    DoTouchEvent(m_PlayerID, m_CapturePointID);
    DoLeaveEvent(m_PlayerID, m_CapturePointID);
    DoTouchEvent(m_PlayerID, m_CapturePointID);
    DoTouchEvent(m_PlayerID, m_CapturePointID3);
}
void CapturePointTest::TestSetup2_TwoPlayersOnCapturePoint()
{
    Events::TriggerTouch touchEvent;
    Events::TriggerLeave leaveEvent;

    //player touches,leaves m_CapturePointID. and enters m_CapturePointID3
    DoTouchEvent(m_PlayerID, m_CapturePointID);
    DoLeaveEvent(m_PlayerID, m_CapturePointID);
    DoTouchEvent(m_PlayerID, m_CapturePointID3);

    //player2 touches m_CapturePointID,m_CapturePointID2
    DoTouchEvent(m_PlayerID2, m_CapturePointID);
    DoTouchEvent(m_PlayerID2, m_CapturePointID2);
}
void CapturePointTest::TestSetup3_NoPlayersOnCapturePoint()
{
    Events::TriggerTouch touchEvent;
    Events::TriggerLeave leaveEvent;

    //player1 touches and leaves m_CapturePointID
    DoTouchEvent(m_PlayerID, m_CapturePointID);
    DoLeaveEvent(m_PlayerID, m_CapturePointID);

    //player2 touches and leaves m_CapturePointID2
    DoTouchEvent(m_PlayerID2, m_CapturePointID2);
    DoLeaveEvent(m_PlayerID2, m_CapturePointID2);
}
void CapturePointTest::TestSetup4_TwoCapturePointsBeingCaptured()
{
    Events::TriggerTouch touchEvent;

    //player1 touches m_CapturePointID3
    DoTouchEvent(m_PlayerID, m_CapturePointID3);

    //player2 touches m_CapturePointID
    DoTouchEvent(m_PlayerID2, m_CapturePointID);
}
void CapturePointTest::TestSetup5_SameCapturePointContestedAndTakenOver()
{
    //NOTE: setup events need to trigger first then the real event will be allowed by the system later

    //"SETUP" homebase->same capturep
    //player1 touches m_CapturePointID3
    DoTouchEvent(m_PlayerID, m_CapturePointID3);

    //player2 touches m_CapturePointID
    DoTouchEvent(m_PlayerID2, m_CapturePointID);

    //contested same, player1 touches the contested
    //player1 touches m_CapturePointID2
    DoTouchEvent(m_PlayerID, m_CapturePointID2);
}
void CapturePointTest::TestSetup6_Team1CapturedTheLastPointAndWon()
{
    //player1 touches m_CapturePointID3
    DoTouchEvent(m_PlayerID, m_CapturePointID3);

    //TODO: this should be in UPDATE instead

    //player1 touches m_CapturePointID2
    DoTouchEvent(m_PlayerID, m_CapturePointID2);

    //player1 touches m_CapturePointID
    DoTouchEvent(m_PlayerID, m_CapturePointID);

    //player2 does nothing
}
void CapturePointTest::TestSetup7()
{
    //2 owns 1
    DoTouchEvent(m_PlayerID2, m_CapturePointID);
    //1 owns 3
    DoTouchEvent(m_PlayerID, m_CapturePointID3);
}
void CapturePointTest::TestSetup8()
{
    //2 owns 3
    DoTouchEvent(m_PlayerID2, m_CapturePointID3);
    //1 owns 1
    DoTouchEvent(m_PlayerID, m_CapturePointID);
}
void CapturePointTest::DoTouchEvent(EntityID whoDidSomething, EntityID onWhatObject) {
    Events::TriggerTouch touchEvent;
    touchEvent.Entity = whoDidSomething;
    touchEvent.Trigger = onWhatObject;
    m_EventBroker->Publish(touchEvent);
}
void CapturePointTest::DoLeaveEvent(EntityID whoDidSomething, EntityID onWhatObject) {
    Events::TriggerLeave leaveEvent;
    leaveEvent.Entity = whoDidSomething;
    leaveEvent.Trigger = onWhatObject;
    m_EventBroker->Publish(leaveEvent);
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
void CapturePointTest::TestSuccess7() {
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "CapturePoint")["OwnedBy"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "CapturePoint")["OwnedBy"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "CapturePoint")["OwnedBy"];

    if (NumLoops < 20 && ownedByID1 == 2 & ownedByID3 == 1) {
        phase1Success = true;
    }
    if (NumLoops < 40 && NumLoops > 20 && ownedByID2 == 1) {
        phase2Success = true;
    }
    if (NumLoops < 60 && NumLoops > 40 && ownedByID1 == 1) {
        phase3Success = true;
    }
    if (NumLoops < 90 && NumLoops > 60 && ownedByID2 != 2) {
        phase4Success = true;
    }

    if (NumLoops == 99) {
        if (phase1Success && phase2Success && phase3Success &&phase4Success)
            TestSucceeded = true;
    }
}
void CapturePointTest::TestSuccess8() {
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "CapturePoint")["OwnedBy"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "CapturePoint")["OwnedBy"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "CapturePoint")["OwnedBy"];

    if (NumLoops < 20 && ownedByID3 == 2 & ownedByID1 == 1) {
        phase1Success = true;
    }
    if (NumLoops < 40 && NumLoops > 20 && ownedByID2 == 1) {
        phase2Success = true;
    }
    if (NumLoops < 60 && NumLoops > 40 && ownedByID3 == 1) {
        phase3Success = true;
    }
    if (NumLoops < 90 && NumLoops > 60 && ownedByID2 != 2) {
        phase4Success = true;
    }

    if (NumLoops == 99) {
        if (phase1Success && phase2Success && phase3Success &&phase4Success)
            TestSucceeded = true;
    }
}
void CapturePointTest::UpdateTest7() {
    //loop 1 = team1 has 3, team 2 has 1
    //loop 20 = team1 takes 2, team 1 leaves 1 -> team1 next = 1, team2 next = still 2
    if (NumLoops == 20) {
        //leave previous
        DoLeaveEvent(m_PlayerID2, m_CapturePointID);
        DoLeaveEvent(m_PlayerID, m_CapturePointID3);

        DoTouchEvent(m_PlayerID, m_CapturePointID2);
    }
    //loop 40 = team1 takes 1, team2:s next cap point should now be 1 (instead of 2)
    if (NumLoops == 40) {
        //leave previous, take next
        DoLeaveEvent(m_PlayerID, m_CapturePointID2);
        DoTouchEvent(m_PlayerID, m_CapturePointID);
    }
    //loop 60 = team2 tries to take 2, this shouldnt work now
    if (NumLoops == 60) {
        DoLeaveEvent(m_PlayerID, m_CapturePointID);
        DoTouchEvent(m_PlayerID2, m_CapturePointID2);
    }
}
void CapturePointTest::UpdateTest8() {
    //2 owns 3
    //1 owns 1

    //loop 20 = team1 takes 2, team 1 leaves 1 -> team1 next = 1, team2 next = still 2
    if (NumLoops == 20) {
        //leave previous
        DoLeaveEvent(m_PlayerID2, m_CapturePointID3);
        DoLeaveEvent(m_PlayerID, m_CapturePointID);

        DoTouchEvent(m_PlayerID, m_CapturePointID2);
    }
    //loop 40 = team1 takes 3, team2:s next cap point should now be 1 (instead of 2)
    if (NumLoops == 40) {
        //leave previous, take next
        DoLeaveEvent(m_PlayerID, m_CapturePointID2);
        DoTouchEvent(m_PlayerID, m_CapturePointID3);
    }
    //loop 60 = team2 tries to take 2, this shouldnt work now
    if (NumLoops == 60) {
        DoLeaveEvent(m_PlayerID, m_CapturePointID3);
        DoTouchEvent(m_PlayerID2, m_CapturePointID2);
    }
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
    case 7:
        TestSuccess7();
        UpdateTest7();
        break;
    case 8:
        TestSuccess8();
        UpdateTest8();
        break;
    default:
        break;
    }
}
