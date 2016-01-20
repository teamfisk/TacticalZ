#ifndef CapturePointTest_h__
#define CapturePointTest_h__

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

class CapturePointTest
{
public:
    CapturePointTest(int runTestNumber);
    ~CapturePointTest();

    void Tick();
    bool TestSucceeded = false;
    int NumLoops = 0;

    bool CapturePoint_Game_Loop_OneHundredTimes();

    void TestSetup1_OnePlayerOnCapturePoint();
    void TestSetup2_TwoPlayersOnCapturePoint();
    void TestSetup3_NoPlayersOnCapturePoint();
    void TestSetup4_TwoCapturePointsBeingCaptured();
    void TestSetup5_SameCapturePointContestedAndTakenOver();
    void TestSetup6_Team1CapturedTheLastPointAndWon();
    void TestSetup7();
    void TestSetup8();
    void DoTouchEvent(EntityID whoDidSomething, EntityID onWhatObject);
    void DoLeaveEvent(EntityID whoDidSomething, EntityID onWhatObject);
    void TestSuccess1();
    void TestSuccess2();
    void TestSuccess3();
    void TestSuccess4();
    void TestSuccess5();
    void TestSuccess6();
    void TestSuccess7();
    void TestSuccess8();
    void UpdateTest7();
    void UpdateTest8();

private:
    double m_LastTime;
    ConfigFile* m_Config = nullptr;
    EventBroker* m_EventBroker;
    World* m_World;
    SystemPipeline* m_SystemPipeline;
    EntityID m_RedTeamPlayer, m_BlueTeamPlayer, m_CapturePointID0, m_CapturePointID1, m_CapturePointID2, m_CapturePointID3, m_CapturePointID4;
    int m_RunTestNumber;
    int m_RedTeam, m_BlueTeam;
};

#endif
