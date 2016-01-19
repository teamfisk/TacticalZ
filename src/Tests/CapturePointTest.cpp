#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "CapturePointTest.h"
#include "Game/Systems/HealthSystem.h"

#include "Collision/TriggerSystem.h"
#include "Collision/CollisionSystem.h"
#include "Core/EntityFileWriter.h"
#include "Game/Systems/CapturePointSystem.h"

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
    LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

    // Create the core event broker
    m_EventBroker = new EventBroker();

    // Create a world
    m_World = new World();

    // Create system pipeline
    m_SystemPipeline = new SystemPipeline(m_EventBroker);
    m_SystemPipeline->AddSystem<HealthSystem>(0);
    m_SystemPipeline->AddSystem<CapturePointSystem>(1);

    //must register components (Components.xsd), else you cant create entities. Easiest done by loading a test xsd file
    auto file = ResourceManager::Load<EntityFile>("Schema/Entities/TeamTest.xml");
    EntityFilePreprocessor fpp(file);
    fpp.RegisterComponents(m_World);
    EntityFileParser fp(file);
    fp.MergeEntities(m_World);

    /*
    ---TESTSETUP---
    default: 2 players
    healthcomponent
    3 capturepoints
    capturepoint(1) = home for team number 2
    capturepoint3 = home for team number 1
    */
    EntityID playerID = m_World->CreateEntity();
    m_RedTeamPlayer = playerID;
    ComponentWrapper& player = m_World->AttachComponent(m_RedTeamPlayer, "Player");
    ComponentWrapper& health = m_World->AttachComponent(m_RedTeamPlayer, "Health");
    ComponentWrapper& playerTeam = m_World->AttachComponent(m_RedTeamPlayer, "Team");
    playerTeam["Team"] = playerTeam["Team"].Enum("Red");
    m_RedTeam = playerTeam["Team"].Enum("Red");
    m_BlueTeam = playerTeam["Team"].Enum("Blue");

    EntityID playerID2 = m_World->CreateEntity();
    m_BlueTeamPlayer = playerID2;
    ComponentWrapper& player2 = m_World->AttachComponent(m_BlueTeamPlayer, "Player");
    ComponentWrapper& health2 = m_World->AttachComponent(m_BlueTeamPlayer, "Health");
    ComponentWrapper& playerTeam2 = m_World->AttachComponent(m_BlueTeamPlayer, "Team");
    playerTeam2["Team"] = m_BlueTeam;

    EntityID capturePointID = m_World->CreateEntity();
    m_CapturePointID = capturePointID;
    ComponentWrapper& capturePoint = m_World->AttachComponent(capturePointID, "CapturePoint");
    ComponentWrapper& capturePointHomeTeam = m_World->AttachComponent(capturePointID, "Team");
    //this capturePoint is homeBase for team 2
    capturePointHomeTeam["Team"] = m_BlueTeam;
    capturePoint["CapturePointNumber"] = 0;

    EntityID capturePointID2 = m_World->CreateEntity();
    m_CapturePointID2 = capturePointID2;
    ComponentWrapper& capturePoint2 = m_World->AttachComponent(capturePointID2, "CapturePoint");
    //ComponentWrapper& capturePointHomeTeam2 = m_World->AttachComponent(capturePointID2, "Team");
    //capturePointHomeTeam2["Team"] = m_BlueTeam;
    capturePoint2["CapturePointNumber"] = 1;

    EntityID capturePointID3 = m_World->CreateEntity();
    m_CapturePointID3 = capturePointID3;
    ComponentWrapper& capturePoint3 = m_World->AttachComponent(capturePointID3, "CapturePoint");
    ComponentWrapper& capturePointHomeTeam3 = m_World->AttachComponent(capturePointID3, "Team");
    //this capturePoint is homeBase for team 1
    capturePointHomeTeam3["Team"] = m_RedTeam;
    capturePoint3["CapturePointNumber"] = 2;

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
        capturePointHomeTeam["Team"] = m_RedTeam;
        capturePointHomeTeam3["Team"] = m_BlueTeam;
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
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID);
    DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
}
void CapturePointTest::TestSetup2_TwoPlayersOnCapturePoint()
{
    Events::TriggerTouch touchEvent;
    Events::TriggerLeave leaveEvent;

    //player touches,leaves m_CapturePointID. and enters m_CapturePointID3
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID);
    DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);

    //player2 touches m_CapturePointID,m_CapturePointID2
    DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID);
    DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID2);
}
void CapturePointTest::TestSetup3_NoPlayersOnCapturePoint()
{
    Events::TriggerTouch touchEvent;
    Events::TriggerLeave leaveEvent;
    DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID);
    DoLeaveEvent(m_BlueTeamPlayer, m_CapturePointID3);
}
void CapturePointTest::TestSetup4_TwoCapturePointsBeingCaptured()
{
    Events::TriggerTouch touchEvent;

    //player1 touches m_CapturePointID3
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);

    //player2 touches m_CapturePointID
    DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID);
}
void CapturePointTest::TestSetup5_SameCapturePointContestedAndTakenOver()
{
    //contested same, player1 touches the contested
    //player1 touches m_CapturePointID2
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID2);
}
void CapturePointTest::TestSetup6_Team1CapturedTheLastPointAndWon()
{
    //player1 touches m_CapturePointID3
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);

    //TODO: this should be in UPDATE instead

    //player1 touches m_CapturePointID2
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID2);

    //player1 touches m_CapturePointID
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID);

    //player2 does nothing
}
void CapturePointTest::TestSetup7()
{
    //2 owns 1
    DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID);
    //1 owns 3
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
}
void CapturePointTest::TestSetup8()
{
    //2 owns 3
    DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID3);
    //1 owns 1
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID);
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
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    if (ownedByID3 == m_RedTeam)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess2() {
    //TestSetup2_TwoPlayersOnCapturePoint
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "Team")["Team"];
    if (ownedByID3 == m_RedTeam && ownedByID1 == m_BlueTeam)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess3() {
    //TestSetup3_NoPlayersOnCapturePoint
    //only do this test if were at the final loopcount
    //if any capturePoint changed then, its a failure else a success
    if (NumLoops == 95) {
        TestSucceeded = true;
        int ownedByID1 = m_World->GetComponent(m_CapturePointID, "Team")["Team"];
        int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
        int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
        if (ownedByID1 != m_BlueTeam || ownedByID2 == m_RedTeam || ownedByID2 == m_BlueTeam || ownedByID3 !=m_RedTeam)
            TestSucceeded = false;
    }
}
void CapturePointTest::TestSuccess4() {
    //TestSetup4_TwoCapturePointsBeingCaptured
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "Team")["Team"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    if (ownedByID1 == m_BlueTeam && ownedByID3 == m_RedTeam)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess5() {
    //TestSetup5_SameCapturePointContestedAndTakenOver
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "Team")["Team"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    if (ownedByID1 == m_BlueTeam && ownedByID2 == m_RedTeam && ownedByID3 == m_RedTeam)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess6() {
    //NOTE: the actual win-event will have to be manually checked if it triggered or not
    //TestSetup6_Team1CapturedTheLastPointAndWon
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "Team")["Team"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    if (ownedByID1 == m_RedTeam && ownedByID2 == m_RedTeam && ownedByID3 == m_RedTeam)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess7() {
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "Team")["Team"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];

    if (NumLoops < 20 && ownedByID1 == m_BlueTeam & ownedByID3 == m_RedTeam) {
        phase1Success = true;
    }
    if (NumLoops < 40 && NumLoops > 20 && ownedByID2 == m_RedTeam) {
        phase2Success = true;
    }
    if (NumLoops < 60 && NumLoops > 40 && ownedByID1 == m_RedTeam) {
        phase3Success = true;
    }
    if (NumLoops < 90 && NumLoops > 60 && ownedByID2 != m_BlueTeam) {
        phase4Success = true;
    }

    if (NumLoops == 99) {
        if (phase1Success && phase2Success && phase3Success &&phase4Success)
            TestSucceeded = true;
    }
}
void CapturePointTest::TestSuccess8() {
    int ownedByID1 = m_World->GetComponent(m_CapturePointID, "CapturePoint")["Team"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "CapturePoint")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "CapturePoint")["Team"];

    if (NumLoops < 20 && ownedByID3 == m_BlueTeam & ownedByID1 == m_RedTeam) {
        phase1Success = true;
    }
    if (NumLoops < 40 && NumLoops > 20 && ownedByID2 == m_RedTeam) {
        phase2Success = true;
    }
    if (NumLoops < 60 && NumLoops > 40 && ownedByID3 == m_RedTeam) {
        phase3Success = true;
    }
    if (NumLoops < 90 && NumLoops > 60 && ownedByID2 != m_BlueTeam) {
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
        DoLeaveEvent(m_BlueTeamPlayer, m_CapturePointID);
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID3);

        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID2);
    }
    //loop 40 = team1 takes 1, team2:s next cap point should now be 1 (instead of 2)
    if (NumLoops == 40) {
        //leave previous, take next
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID2);
        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID);
    }
    //loop 60 = team2 tries to take 2, this shouldnt work now
    if (NumLoops == 60) {
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID);
        DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID2);
    }
}
void CapturePointTest::UpdateTest8() {
    //2 owns 3
    //1 owns 1

    //loop 20 = team1 takes 2, team 1 leaves 1 -> team1 next = 1, team2 next = still 2
    if (NumLoops == 20) {
        //leave previous
        DoLeaveEvent(m_BlueTeamPlayer, m_CapturePointID3);
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID);

        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID2);
    }
    //loop 40 = team1 takes 3, team2:s next cap point should now be 1 (instead of 2)
    if (NumLoops == 40) {
        //leave previous, take next
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID2);
        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
    }
    //loop 60 = team2 tries to take 2, this shouldnt work now
    if (NumLoops == 60) {
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID3);
        DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID2);
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
