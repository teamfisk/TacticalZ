#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "Common.h"
#include "GLM.h"
#include "Core/World.h"

BOOST_AUTO_TEST_CASE(WorldTestSingleAllocation, * boost::unit_test::tolerance(0.001))
{
    World w;

    auto f = ComponentWrapperFactory("Test");
    f.AddProperty("TestInteger", 1337);
    f.AddProperty("TestDouble", 13.37);
    f.AddProperty("TestString", std::string("Carlito"));
    f.AddProperty("TestVec3", glm::vec3(1.f, 2.f, 3.f));
    w.RegisterComponent(f);

    EntityID e = w.CreateEntity();
    ComponentWrapper c = w.AttachComponent(e, "Test");

    // Check default values
    BOOST_TEST((int)c["TestInteger"] == c.Field<int>("TestInteger"));
    BOOST_TEST((int)c["TestInteger"] == 1337);
    BOOST_TEST((double)c["TestDouble"] == 13.37);
    BOOST_TEST((std::string)c["TestString"] == "Carlito");
    glm::vec3 vec3 = c["TestVec3"];
    BOOST_TEST(vec3.x == 1.f);
    BOOST_TEST(vec3.y == 2.f);
    BOOST_TEST(vec3.z == 3.f);

    // Change values
    ((int&)c["TestInteger"]) += 1;
    BOOST_TEST((int)c["TestInteger"] == 1338);
    ((double&)c["TestDouble"]) += 1.11;
    std::cout << (double)c["TestDouble"] << std::endl;
    BOOST_TEST((double)c["TestDouble"] == 14.48);
    c["TestString"] = "Siesta";
    BOOST_TEST((std::string)c["TestString"] == "Siesta");
    ((glm::vec3&)c["TestVec3"]).y += 1.f;
    BOOST_TEST(((glm::vec3)c["TestVec3"]).y == 3.f);
}

BOOST_AUTO_TEST_CASE(WorldTestMultipleAllocations, * utf::tolerance(0.00001))
{
    World w;

    // Create allocation for 3 entities
    auto f = ComponentWrapperFactory("Test", 3);
    f.AddProperty("TestInteger", 1337);
    f.AddProperty("TestDouble", 13.37);
    f.AddProperty("TestString", std::string("Carlito"));
    f.AddProperty("TestVec3", glm::vec3(1.f, 2.f, 3.f));
    w.RegisterComponent(f);

    // Create 6 entities with Test components
    // 3 will reside in contiguous memory
    // 3 will be allocated dynamically
    for (int i = 0; i < 6; i++) {
        EntityID e = w.CreateEntity();
        ComponentWrapper c = w.AttachComponent(e, "Test");
        c["TestInteger"] = i;
    }

    // Loop through them and check data
    int i = 0;
    for (auto& c : *w.GetComponents("Test")) {
        BOOST_TEST((int)c["TestInteger"] == i);
        i++;
    }
}

BOOST_AUTO_TEST_CASE(WorldCopy, *utf::tolerance(0.00001))
{
    World w1;

    // Create a test component
    auto testComponent = ComponentWrapperFactory("Test", 2);
    testComponent.AddProperty("TestInteger", 1337);
    testComponent.AddProperty("TestDouble", 13.37);
    testComponent.AddProperty("TestString", std::string("DefaultString"));
    testComponent.AddProperty("TestVec3", glm::vec3(1.f, 2.f, 3.f));
    w1.RegisterComponent(testComponent);

    // Create a test entity
    EntityID w1_e1 = w1.CreateEntity();
    auto w1_c1 = w1.AttachComponent(w1_e1, "Test");

    // Create a child
    EntityID w1_e2 = w1.CreateEntity(w1_e1);
    auto w1_c2 = w1.AttachComponent(w1_e2, "Test");
    w1_c2["TestString"] = "NonDefaultString";

    // Copy the world!
    World w2 = w1;

    // Fetch the components
    auto w2_c1 = w2.GetComponent(w1_e1, "Test");
    auto w2_c2 = w2.GetComponent(w1_e2, "Test");

    // Check that built-in types are copied but don't reside in the same memory
    BOOST_CHECK((int)w1_c1["TestInteger"] == (int)w2_c1["TestInteger"]);
    BOOST_CHECK(&(int&)w1_c1["TestInteger"] != &(int&)w2_c1["TestInteger"]);
    BOOST_CHECK((double)w1_c1["TestDouble"] == (double)w2_c1["TestDouble"]);
    BOOST_CHECK(&(int&)w1_c1["TestDouble"] != &(int&)w2_c1["TestDouble"]);
    BOOST_CHECK((int)w1_c2["TestInteger"] == (int)w2_c2["TestInteger"]);
    BOOST_CHECK(&(int&)w1_c2["TestInteger"] != &(int&)w2_c2["TestInteger"]);
    BOOST_CHECK((double)w1_c2["TestDouble"] == (double)w2_c2["TestDouble"]);
    BOOST_CHECK(&(int&)w1_c2["TestDouble"] != &(int&)w2_c2["TestDouble"]);
    // Check that specially handled strings are fine
    BOOST_CHECK((std::string)w1_c1["TestString"] == (std::string)w2_c1["TestString"]);
    BOOST_CHECK(&(std::string&)w1_c1["TestString"] != &(std::string&)w2_c1["TestString"]);
    BOOST_CHECK((std::string)w1_c2["TestString"] == (std::string)w2_c2["TestString"]);
    BOOST_CHECK(&(std::string&)w1_c2["TestString"] != &(std::string&)w2_c2["TestString"]);
}
