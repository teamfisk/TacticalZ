#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "HealthSystemTest.h"
#include "Game/Systems/HealthSystem.h"

BOOST_AUTO_TEST_SUITE(HealthSystemSuite)

BOOST_AUTO_TEST_CASE(HealthSystemTest)
{
    //this tests 2 healthevents and the healthsystem
    GameHealthSystemTest game;
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

GameHealthSystemTest::GameHealthSystemTest()
{
    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    ResourceManager::RegisterType<EntityFile>("EntityFile");

    m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
    LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

    // Create the core event broker
    m_EventBroker = new EventBroker();

    // Create a world
    m_World = new World();

    auto file = ResourceManager::Load<EntityFile>("Schema/Entities/TeamTest.xml");
    EntityFilePreprocessor fpp(file);
    fpp.RegisterComponents(m_World);
    EntityFileParser fp(file);
    fp.MergeEntities(m_World);

    // Create system pipeline
    m_SystemPipeline = new SystemPipeline(m_World,m_EventBroker);
    m_SystemPipeline->AddSystem<HealthSystem>(0);

    //The Test
    //create entity which has transform,player,model,health in it. i.e. is a player
    EntityID playerID = m_World->CreateEntity();
    ComponentWrapper transform = m_World->AttachComponent(playerID, "Transform");
    ComponentWrapper model = m_World->AttachComponent(playerID, "Model");
    model["Resource"] = "Models/Core/UnitSphere.mesh"; // 360NoScope UnitSphere
    ComponentWrapper player = m_World->AttachComponent(playerID, "Player");
    ComponentWrapper health = m_World->AttachComponent(playerID, "Health");
    healthsID = playerID;
    double currentHealth = (double)m_World->GetComponent(healthsID, "Health")["Health"];

    //heal player with 40
    Events::PlayerHealthPickup e3;
    e3.HealthAmount = 40.0f;
    e3.PlayerHealedID = healthsID;
    m_EventBroker->Publish(e3);
    //damage player with 50
    Events::PlayerDamage e;
    e.Damage = 50.0f;
//    e.PlayerDamagedID = healthsID;
    m_EventBroker->Publish(e);
    //heal some other player with 40
    Events::PlayerHealthPickup e2;
    e2.HealthAmount = 40.0f;
    e2.PlayerHealedID = healthsID + 1;
    m_EventBroker->Publish(e2);

    EntityID playerID2 = m_World->CreateEntity();
    ComponentWrapper transform2 = m_World->AttachComponent(playerID2, "Transform");
    ComponentWrapper model2 = m_World->AttachComponent(playerID2, "Model");
    model2["Resource"] = "Models/Core/UnitSphere.mesh"; // 360NoScope UnitSphere
    ComponentWrapper player2 = m_World->AttachComponent(playerID2, "Player");
    ComponentWrapper health2 = m_World->AttachComponent(playerID2, "Health");
    //END TEST
}

GameHealthSystemTest::~GameHealthSystemTest()
{
    delete m_SystemPipeline;
    delete m_World;
    delete m_EventBroker;
}

void GameHealthSystemTest::Tick()
{
    glfwPollEvents();

    double currentTime = glfwGetTime();
    double dt = currentTime - m_LastTime;
    m_LastTime = currentTime;

    // Iterate through systems and update world!
    m_SystemPipeline->Update(dt);

    m_EventBroker->Swap();
    m_EventBroker->Clear();

    //if health reaches 90 then we know the test has succeeded (start with 100hp, remove 50hp, add 40hp)
    double currentHealth = (double)m_World->GetComponent(healthsID, "Health")["Health"];
    if (currentHealth == 90)
        TestSucceeded = true;
}
