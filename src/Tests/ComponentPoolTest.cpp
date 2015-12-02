#include <boost/test/unit_test.hpp>

#include "Core/ComponentPool.h"

BOOST_AUTO_TEST_CASE(ComponentPoolTest)
{
    ComponentInfo ci;
    ci.Name = "Test";
    ci.FieldTypes["Field"] = "int";
    ci.FieldOffsets["Field"] = sizeof(EntityID);
    ci.Meta.Allocation = 4;
    ci.Meta.Stride = sizeof(EntityID) + sizeof(int);

    ComponentPool pool(ci);
    for (int i = 0; i < 3; i++) {
        ComponentWrapper c = pool.New();
        c.EntityID = i;
        unsigned int offset = c.Info.FieldOffsets.at("Field");
        memcpy(&c.Data[offset], &i, sizeof(int));
    }

    int i = 0;
    for (auto& c : pool) {
        BOOST_CHECK(c.EntityID == i);
        int field = c.Property<int>("Field");
        BOOST_CHECK(field == i);
        i++;
    }
}