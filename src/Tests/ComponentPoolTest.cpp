#include <boost/test/unit_test.hpp>

#include "Core/ComponentPool.h"

BOOST_AUTO_TEST_CASE(ComponentPoolTest)
{
    // TODO: Write an updated test for component pool
    BOOST_CHECK(false);
    //ComponentInfo ci;
    //ci.Name = "Test";
    //ci.FieldTypes["Field"] = "int";
    //ci.FieldOffsets["Field"] = 0;
    //ci.Meta->Allocation = 3;
    //ci.Stride = sizeof(EntityID) + sizeof(int);

    //std::vector<ComponentWrapper> wrappers;
    //ComponentPool pool(ci);
    //for (int i = 0; i < 4; i++) {
    //    ComponentWrapper c = pool.New();
    //    c.EntityID = i;
    //    unsigned int offset = c.Info.FieldOffsets.at("Field");
    //    memcpy(&c.Data[offset], &i, sizeof(int));
    //    wrappers.push_back(c);
    //}

    //int i = 0;
    //for (auto& c : pool) {
    //    BOOST_CHECK(c.EntityID == i);
    //    BOOST_CHECK((int)c["Field"] == i);
    //    i++;
    //}

    //pool.Delete(wrappers[1]);
}