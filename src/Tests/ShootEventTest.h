#ifndef ShootEventTest_h__
#define ShootEventTest_h__

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
#include "PlayerSystem.h"

#include "Core/EntityFilePreprocessor.h"
#include "Core/EntityFileParser.h"
#include "Core/EntityFileWriter.h"

#include "Core\EMouseRelease.h"
#include "Core\EShoot.h"

class ShootEventTest
{
public:
    ShootEventTest(int runTestNumber);
    ~ShootEventTest();

    void Tick();
    bool TestSucceeded = false;

private:
    void TestSetup1(ComponentWrapper &player, ComponentWrapper &pItem, ComponentWrapper &sItem);
    void TestSetup2(ComponentWrapper &player, ComponentWrapper &pItem, ComponentWrapper &sItem);
    void TestSetup3(ComponentWrapper &player, ComponentWrapper &pItem, ComponentWrapper &sItem);
    void TestSetup4(ComponentWrapper &player, ComponentWrapper &pItem, ComponentWrapper &sItem);
    void TestSuccess1();
    void TestSuccess2();
    void TestSuccess3();
    void TestSuccess4();

    double m_LastTime;
    ConfigFile* m_Config = nullptr;
    EventBroker* m_EventBroker;
    World* m_World;
    SystemPipeline* m_SystemPipeline;
    int m_PlayerID;
    int m_RunTestNumber;

};

#endif
