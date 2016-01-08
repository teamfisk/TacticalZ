#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "ShootEventTest.h"
#include "Core\EPlayerDamage.h";
#include "Core\EPlayerHealthPickup.h";
#include "Core\EPlayerDeath.h";
#include "Game/HealthSystem.h"

BOOST_AUTO_TEST_SUITE(ShootEventTestSuite)

//AShootEventTest != ShootEventTest -> else it confuses names!
BOOST_AUTO_TEST_CASE(AShootEventTest)
{
    ShootEventTest game;
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

ShootEventTest::ShootEventTest()
{
    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    ResourceManager::RegisterType<EntityXMLFile>("EntityXMLFile");

    m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
    LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

    // Create the core event broker
    m_EventBroker = new EventBroker();

     // Create a world
    m_World = new World();
    std::string mapToLoad = m_Config->Get<std::string>("Debug.LoadMap", "");
    if (!mapToLoad.empty()) {
        ResourceManager::Load<EntityXMLFile>(mapToLoad)->PopulateWorld(m_World);
    }

    // Create system pipeline
    m_SystemPipeline = new SystemPipeline(m_EventBroker);
    m_SystemPipeline->AddSystem<PlayerSystem>(0);
    m_SystemPipeline->AddSystem<HealthSystem>(0);

    //The Test
    //create entity which has transorm,player,model,health in it. i.e. is a player
    EntityID playerID = m_World->CreateEntity();
    ComponentWrapper transform = m_World->AttachComponent(playerID, "Transform");
    ComponentWrapper model = m_World->AttachComponent(playerID, "Model");
    model["Resource"] = "Models/Core/UnitSphere.obj";
    ComponentWrapper& player = m_World->AttachComponent(playerID, "Player");
    ComponentWrapper health = m_World->AttachComponent(playerID, "Health");
    playersID = playerID;
    //attach 2x weaps
    ComponentWrapper& pItem = m_World->AttachComponent(playerID, "PrimaryItem");
    ComponentWrapper& sItem= m_World->AttachComponent(playerID, "SecondaryItem");
    //set currentweap
    player["EquippedItem"] = 1;
    //set ammo set cooldown
    pItem["Ammo"] = 100;
    pItem["CoolDownTimer"] = 0.0f;

    //trigger event leftmousedown
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

void ShootEventTest::Tick()
{
    glfwPollEvents();

    double currentTime = glfwGetTime();
    double dt = currentTime - m_LastTime;
    m_LastTime = currentTime;

    // Iterate through systems and update world!
    m_SystemPipeline->Update(m_World, dt);

    m_EventBroker->Swap();
    m_EventBroker->Clear();

    //if ammocount reaches 99 we know the test has succeeded, i.e. a shot has been fired
    int currentAmmo = (int)m_World->GetComponent(playersID, "PrimaryItem")["Ammo"];
    if (currentAmmo ==99)
        TestSucceeded = true;
}
