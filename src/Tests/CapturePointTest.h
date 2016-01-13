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
#include "PlayerSystem.h"

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

private:
    double m_LastTime;
    ConfigFile* m_Config = nullptr;
    EventBroker* m_EventBroker;
    World* m_World;
    SystemPipeline* m_SystemPipeline;
    int m_PlayerID, m_PlayerID2, m_CapturePointID, m_CapturePointID2;
    int m_RunTestNumber;

};

#endif
