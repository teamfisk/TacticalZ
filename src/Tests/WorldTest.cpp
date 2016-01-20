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
