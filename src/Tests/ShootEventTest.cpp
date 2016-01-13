#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "ShootEventTest.h"
#include "Game/HealthSystem.h"

BOOST_AUTO_TEST_SUITE(ShootEventTestSuite)

//dont use the same name as the classname in test cases...
BOOST_AUTO_TEST_CASE(ShootEventTest_PrimaryWeaponFiring)
{
    //Test firing primary weapon
    ShootEventTest game(1);
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
BOOST_AUTO_TEST_CASE(ShootEventTest_SecondaryWeaponFiring)
{
    //Test firing secondary weapon
    ShootEventTest game(2);
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
BOOST_AUTO_TEST_CASE(ShootEventTest_NoWeaponFiring)
{
    //Test firing with no weapon equipped
    ShootEventTest game(3);
    //100 loops will be more than enough to do the test
    int loops = 100;
    bool success = false;
    while (loops > 0) {
        game.Tick();
        loops--;
    }
    //The system will process the events, hence it will take a while before we can read anything
    if (game.TestSucceeded)
        success = true;
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_CASE(ShootEventTest_WeaponOnCooldown)
{
    //Test firing with weapon on cooldown
    ShootEventTest game(4);
    //100 loops will be more than enough to do the test
    int loops = 100;
    bool success = false;
    while (loops > 0) {
        game.Tick();
        loops--;
    }
    //The system will process the events, hence it will take a while before we can read anything
    if (game.TestSucceeded)
        success = true;
    BOOST_TEST(success);
}
BOOST_AUTO_TEST_SUITE_END()

ShootEventTest::ShootEventTest(int runTestNumber)
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
    //attach 2x weaps
    ComponentWrapper& pItem = m_World->AttachComponent(playerID, "PrimaryItem");
    ComponentWrapper& sItem = m_World->AttachComponent(playerID, "SecondaryItem");

    m_RunTestNumber = runTestNumber;
    switch (runTestNumber)
    {
    case 1:
        TestSetup1(player, pItem, sItem);
        break;
    case 2:
        TestSetup2(player, pItem, sItem);
        break;
    case 3:
        TestSetup3(player, pItem, sItem);
        break;
    case 4:
        TestSetup4(player, pItem, sItem);
        break;
    default:
        break;
    }

    //fire once = trigger event leftmousedown
    Events::MouseRelease eMouseRelease;
    eMouseRelease.Button = GLFW_MOUSE_BUTTON_LEFT;
    eMouseRelease.X = 1.0f;
    eMouseRelease.Y = 1.0f;
    m_EventBroker->Publish(eMouseRelease);
}

ShootEventTest::~ShootEventTest()
{
    delete m_SystemPipeline;
    delete m_World;
    delete m_EventBroker;
}

void ShootEventTest::TestSetup1(ComponentWrapper& player, ComponentWrapper& pItem, ComponentWrapper& sItem)
{
    //set currentweap
    player["EquippedItem"] = 1;
    //set ammo set cooldown
    pItem["Ammo"] = 100;
    pItem["CoolDownTimer"] = 0.0;
}
void ShootEventTest::TestSetup2(ComponentWrapper& player, ComponentWrapper& pItem, ComponentWrapper& sItem)
{
    //set currentweap
    player["EquippedItem"] = 2;
    //set ammo set cooldown
    sItem["Ammo"] = 10;
    sItem["CoolDownTimer"] = 0.0;
}
void ShootEventTest::TestSetup3(ComponentWrapper& player, ComponentWrapper& pItem, ComponentWrapper& sItem)
{
    player["EquippedItem"] = 0;
    pItem["Ammo"] = 100;
    sItem["Ammo"] = 100;
    //TestSucceeded will be set to false if ammo changes during the 100 loops
    TestSucceeded = true;
}
void ShootEventTest::TestSetup4(ComponentWrapper& player, ComponentWrapper& pItem, ComponentWrapper& sItem)
{
    //set currentweap
    player["EquippedItem"] = 1;
    //set ammo set cooldown
    pItem["Ammo"] = 100;
    pItem["CoolDownTimer"] = 99999999.0;//very long coolDownTimer
    //TestSucceeded will be set to false if ammo changes during the 100 loops
    TestSucceeded = true;
}
void ShootEventTest::TestSuccess1() {
    //if ammocount reaches -- we know the test has succeeded, i.e. a shot has been fired
    int currentAmmo = m_World->GetComponent(m_PlayerID, "PrimaryItem")["Ammo"];
    if (currentAmmo == 99)
        TestSucceeded = true;
}
void ShootEventTest::TestSuccess2() {
    //if ammocount reaches -- we know the test has succeeded, i.e. a shot has been fired
    int currentAmmo = m_World->GetComponent(m_PlayerID, "SecondaryItem")["Ammo"];
    if (currentAmmo == 9)
        TestSucceeded = true;
}
void ShootEventTest::TestSuccess3() {
    //try firing again
    Events::MouseRelease eMouseRelease;
    eMouseRelease.Button = GLFW_MOUSE_BUTTON_LEFT;
    eMouseRelease.X = 1.0f;
    eMouseRelease.Y = 1.0f;
    m_EventBroker->Publish(eMouseRelease);
    //if ammocount reaches -- we know the test has succeeded, i.e. a shot has been fired
    int currentAmmo = m_World->GetComponent(m_PlayerID, "PrimaryItem")["Ammo"];
    int currentAmmoSecondary = m_World->GetComponent(m_PlayerID, "SecondaryItem")["Ammo"];
    if (currentAmmo != 100 || currentAmmoSecondary != 100)
        TestSucceeded = false;
}
void ShootEventTest::TestSuccess4() {
    //try firing again
    Events::MouseRelease eMouseRelease;
    eMouseRelease.Button = GLFW_MOUSE_BUTTON_LEFT;
    eMouseRelease.X = 1.0f;
    eMouseRelease.Y = 1.0f;
    m_EventBroker->Publish(eMouseRelease);    
    //if ammocount reaches -- we know the test has succeeded, i.e. a shot has been fired
    int currentAmmo = m_World->GetComponent(m_PlayerID, "PrimaryItem")["Ammo"];
    if (currentAmmo != 100)
        TestSucceeded = false;
}
void ShootEventTest::Tick()
{
    glfwPollEvents();

    //double currentTime = glfwGetTime();
    //double dt = currentTime - m_LastTime;
    //m_LastTime = currentTime;

    //just set dt to 1.0
    double dt = 0.34567;
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
    default:
        break;
    }

}
