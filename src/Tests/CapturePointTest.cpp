#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "CapturePointTest.h"
#include "Game/Systems/HealthSystem.h"

#include "Collision/TriggerSystem.h"
#include "Collision/CollisionSystem.h"
#include "Core/EntityXMLFileWriter.h"
#include "Game/Systems/CapturePointSystem.h"

BOOST_AUTO_TEST_SUITE(CapturePointTestSuite)

//dont use the same name as the classname in test cases...
BOOST_AUTO_TEST_CASE(CapturePointTest1_OnePlayerOnCapturePoint)
{
    CapturePointTest game(1);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest2_TwoPlayersOnCapturePoint)
{
    CapturePointTest game(2);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest3_NoPlayersOnCapturePoint)
{
    CapturePointTest game(3);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest4_TwoCapturePointsBeingCaptured)
{
    CapturePointTest game(4);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest5_SameCapturePointContestedAndTakenOver)
{
    CapturePointTest game(5);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest6_Team1CapturedTheLastPointAndWon)
{
    CapturePointTest game(6);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest7_Team1ForcesTeam2sNextCapturePointToGoBackwards1Step)
{
    CapturePointTest game(7);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(CapturePointTest8_Team2ForcesTeam1sNextCapturePointToGoForwards1Step)
{
    CapturePointTest game(8);
    bool success = game.CapturePoint_Game_Loop_OneHundredTimes();
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_SUITE_END()

bool CapturePointTest::CapturePoint_Game_Loop_OneHundredTimes() {
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
    ResourceManager::RegisterType<EntityXMLFile>("EntityFile");

    m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
    LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

    // Create the core event broker
    m_EventBroker = new EventBroker();

    // Create a world
    m_World = new World();

    // Create system pipeline
    m_SystemPipeline = new SystemPipeline(m_World,m_EventBroker, true, false);
    m_SystemPipeline->AddSystem<HealthSystem>(0);
    m_SystemPipeline->AddSystem<CapturePointSystem>(1);

    //must register components (Components.xsd), else you cant create entities. Easiest done by loading a test xsd file
    auto file = ResourceManager::Load<EntityXMLFile>("Schema/Entities/TeamTest.xml");
    EntityXMLFilePreprocessor fpp(file);
    fpp.RegisterComponents(m_World);
    EntityXMLFileParser fp(file);
    fp.MergeEntities(m_World);

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

    EntityID capturePointID0 = m_World->CreateEntity();
    m_CapturePointID0 = capturePointID0;
    ComponentWrapper& capturePoint0 = m_World->AttachComponent(capturePointID0, "CapturePoint");
    ComponentWrapper& capturePointTeamOwner0 = m_World->AttachComponent(capturePointID0, "Team");

    capturePointTeamOwner0["Team"] = m_BlueTeam;
    capturePoint0["CapturePointNumber"] = 0;
    capturePoint0["HomePointForTeam"] = m_BlueTeam;

    EntityID capturePointID1 = m_World->CreateEntity();
    m_CapturePointID1 = capturePointID1;
    ComponentWrapper& capturePoint1 = m_World->AttachComponent(capturePointID1, "CapturePoint");
    ComponentWrapper& capturePointTeamOwner1 = m_World->AttachComponent(capturePointID1, "Team");
    capturePoint1["CapturePointNumber"] = 1;
    capturePointTeamOwner1["Team"] = 0;

    EntityID capturePointID2 = m_World->CreateEntity();
    m_CapturePointID2 = capturePointID2;
    ComponentWrapper& capturePoint2 = m_World->AttachComponent(capturePointID2, "CapturePoint");
    ComponentWrapper& capturePointTeamOwner2 = m_World->AttachComponent(capturePointID2, "Team");
    capturePoint2["CapturePointNumber"] = 2;
    capturePointTeamOwner2["Team"] = 0;

    EntityID capturePointID3 = m_World->CreateEntity();
    m_CapturePointID3 = capturePointID3;
    ComponentWrapper& capturePoint3 = m_World->AttachComponent(capturePointID3, "CapturePoint");
    ComponentWrapper& capturePointTeamOwner3 = m_World->AttachComponent(capturePointID3, "Team");
    capturePoint3["CapturePointNumber"] = 3;
    capturePointTeamOwner3["Team"] = 0;

    EntityID capturePointID4 = m_World->CreateEntity();
    m_CapturePointID4 = capturePointID4;
    ComponentWrapper& capturePoint4 = m_World->AttachComponent(capturePointID4, "CapturePoint");
    ComponentWrapper& capturePointTeamOwner4 = m_World->AttachComponent(capturePointID4, "Team");
    capturePointTeamOwner4["Team"] = m_RedTeam;
    capturePoint4["CapturePointNumber"] = 4;
    capturePoint4["HomePointForTeam"] = m_RedTeam;

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
        capturePoint0["HomePointForTeam"] = m_RedTeam;
        capturePointTeamOwner0["Team"] = m_RedTeam;
        capturePoint4["HomePointForTeam"] = m_BlueTeam;
        capturePointTeamOwner4["Team"] = m_BlueTeam;
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
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
}
void CapturePointTest::TestSetup2_TwoPlayersOnCapturePoint()
{
    DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID1);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
    //contested point
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID2);
    DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID2);
}
void CapturePointTest::TestSetup3_NoPlayersOnCapturePoint()
{
    DoLeaveEvent(m_BlueTeamPlayer, m_CapturePointID0);
    DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID4);
}
void CapturePointTest::TestSetup4_TwoCapturePointsBeingCaptured()
{
    //blue = 0
    DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID1);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
}
void CapturePointTest::TestSetup5_SameCapturePointContestedAndTakenOver()
{
    DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID1);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
    //contested point
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID2);
}
void CapturePointTest::TestSetup6_Team1CapturedTheLastPointAndWon()
{
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID4);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID2);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID1);
    DoTouchEvent(m_RedTeamPlayer, m_CapturePointID0);
}
void CapturePointTest::TestSetup7()
{
}
void CapturePointTest::TestSetup8()
{
}
void CapturePointTest::DoTouchEvent(EntityID whoDidSomething, EntityID onWhatObject) {
    Events::TriggerTouch touchEvent;
    touchEvent.Entity = EntityWrapper(m_World, whoDidSomething);
    touchEvent.Trigger = EntityWrapper(m_World, onWhatObject);
    m_EventBroker->Publish(touchEvent);
}
void CapturePointTest::DoLeaveEvent(EntityID whoDidSomething, EntityID onWhatObject) {
    Events::TriggerLeave leaveEvent;
    leaveEvent.Entity = EntityWrapper(m_World, whoDidSomething);
    leaveEvent.Trigger = EntityWrapper(m_World, onWhatObject);
    m_EventBroker->Publish(leaveEvent);
}
void CapturePointTest::TestSuccess1() {
    //TestSetup1_OnePlayerOnCapturePoint
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    if (ownedByID3 == m_RedTeam)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess2() {
    //TestSetup2_TwoPlayersOnCapturePoint
    if (NumLoops == 95) {
        int ownedByID1 = m_World->GetComponent(m_CapturePointID1, "Team")["Team"];
        int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
        int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
        if (ownedByID1 == m_BlueTeam && ownedByID2 == 0 && ownedByID3 == m_RedTeam)
            TestSucceeded = true;
    }
}
void CapturePointTest::TestSuccess3() {
    //TestSetup3_NoPlayersOnCapturePoint
    if (NumLoops == 95) {
        TestSucceeded = true;
        int ownedByID1 = m_World->GetComponent(m_CapturePointID1, "Team")["Team"];
        int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
        int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
        if (ownedByID1 == 0 && ownedByID2 == 0 && ownedByID3 == 0)
            TestSucceeded = true;
    }
}
void CapturePointTest::TestSuccess4() {
    //blue = 0
    //TestSetup4_TwoCapturePointsBeingCaptured
    int ownedByID1 = m_World->GetComponent(m_CapturePointID1, "Team")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    if (ownedByID1 == m_BlueTeam && ownedByID3 == m_RedTeam)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess5() {
    //blue = 0
    //TestSetup5_SameCapturePointContestedAndTakenOver
    int ownedByID1 = m_World->GetComponent(m_CapturePointID1, "Team")["Team"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    if (ownedByID3 == m_RedTeam && ownedByID2 == m_RedTeam && ownedByID1 == m_BlueTeam)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess6() {
    //NOTE: the actual win-event will have to be manually checked if it triggered or not
    //TestSetup6_Team1CapturedTheLastPointAndWon
    int ownedByID0 = m_World->GetComponent(m_CapturePointID0, "Team")["Team"];
    int ownedByID1 = m_World->GetComponent(m_CapturePointID1, "Team")["Team"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    int ownedByID4 = m_World->GetComponent(m_CapturePointID4, "Team")["Team"];
    if (ownedByID1 == m_RedTeam && ownedByID2 == m_RedTeam && ownedByID3 == m_RedTeam && ownedByID4 == m_RedTeam)
        TestSucceeded = true;
}
void CapturePointTest::TestSuccess7() {
    //blue = 0
    int ownedByID0 = m_World->GetComponent(m_CapturePointID0, "Team")["Team"];
    int ownedByID1 = m_World->GetComponent(m_CapturePointID1, "Team")["Team"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    int ownedByID4 = m_World->GetComponent(m_CapturePointID4, "Team")["Team"];

    //    //red has 1,2,3,4 - blue tries to take 2... when it only has 0

    if (NumLoops == 99) {
        if (ownedByID0 == m_BlueTeam && ownedByID1 == m_RedTeam && ownedByID2 == m_RedTeam && ownedByID3 == m_RedTeam && ownedByID4 == m_RedTeam)
            TestSucceeded = true;
    }
}
void CapturePointTest::TestSuccess8() {
    //blue = 4
    int ownedByID0 = m_World->GetComponent(m_CapturePointID0, "Team")["Team"];
    int ownedByID1 = m_World->GetComponent(m_CapturePointID1, "Team")["Team"];
    int ownedByID2 = m_World->GetComponent(m_CapturePointID2, "Team")["Team"];
    int ownedByID3 = m_World->GetComponent(m_CapturePointID3, "Team")["Team"];
    int ownedByID4 = m_World->GetComponent(m_CapturePointID4, "Team")["Team"];

    //red has 0,1,2,3 - blue tries to take 2... when it only has 0
    if (NumLoops == 99) {
        if (ownedByID0 == m_RedTeam && ownedByID1 == m_RedTeam && ownedByID2 == m_RedTeam && ownedByID3 == m_RedTeam && ownedByID4 == m_BlueTeam)
            TestSucceeded = true;
    }
}
void CapturePointTest::UpdateTest7() {
    //blue = 0
    if (NumLoops == 20) {
        //leave previous
        DoLeaveEvent(m_BlueTeamPlayer, m_CapturePointID0);
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID4);

        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
        DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID1);
    }
    if (NumLoops == 40) {
        //leave previous, take next
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID3);
        DoLeaveEvent(m_BlueTeamPlayer, m_CapturePointID1);
        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID2);
        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID1);
    }
    //red has 1,2,3,4 - blue tries to take 2... when it only has 0
    if (NumLoops == 60) {
        DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID2);
    }
}
void CapturePointTest::UpdateTest8() {
    //blue = 4
    if (NumLoops == 20) {
        //leave previous
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID0);
        DoLeaveEvent(m_BlueTeam, m_CapturePointID4);

        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID1);
        DoTouchEvent(m_BlueTeamPlayer, m_CapturePointID3);
    }
    if (NumLoops == 40) {
        //leave previous, take next
        DoLeaveEvent(m_RedTeamPlayer, m_CapturePointID1);
        DoLeaveEvent(m_BlueTeamPlayer, m_CapturePointID3);
        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID2);
        DoTouchEvent(m_RedTeamPlayer, m_CapturePointID3);
    }
    //red has 0,1,2,3 - blue tries to take 2... when it only has 0
    if (NumLoops == 60) {
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
    m_SystemPipeline->Update(dt);

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
