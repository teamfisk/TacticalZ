#include <boost/test/unit_test.hpp>

#include "Common.h"
#include "Core/World.h"

BOOST_AUTO_TEST_CASE(WorldTest)
{
    ComponentInfo ci;
    ci.Name = "Test";
    ci.FieldTypes["Field"] = "int";
    ci.FieldOffsets["Field"] = 0;
    ci.Meta.Stride = sizeof(int);
    ci.Meta.Allocation = 3;

    ci.Defaults = std::shared_ptr<char>(new char[ci.Meta.Stride]);
    int default_Field = 1337;
    memcpy(ci.Defaults.get(), &default_Field, ci.Meta.Stride);

    World w;
    w.RegisterComponent(ci);

    std::vector<EntityID> ids;
    for (int i = 0; i < 6; i++) {
        EntityID e = w.CreateEntity();
        ids.push_back(e);
        w.AttachComponent(e, "Test");
        ComponentWrapper c = w.GetComponent(e, "Test");
        BOOST_CHECK(c.EntityID == e);
        BOOST_CHECK((int)c["Field"] == 1337);
        c.SetProperty("Field", i);
        BOOST_CHECK((int)c["Field"] == i);
    }

    int i = 0;
    for (auto& c : w.GetComponents("Test")) {
        EntityID e = ids.at(i);
        BOOST_CHECK(c.EntityID == e);
        BOOST_CHECK((int)c["Field"] == i);
        i++;
    }
}
