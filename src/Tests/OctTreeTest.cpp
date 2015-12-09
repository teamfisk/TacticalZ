#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;
#include <stdlib.h>//srand
#include <Engine\Core\OctTree.h>

BOOST_AUTO_TEST_SUITE(octTreeTests)

BOOST_AUTO_TEST_CASE(octTreeTest)
{

}

BOOST_AUTO_TEST_CASE(octTreeTest2)
{

}

BOOST_AUTO_TEST_CASE(octSameRegionTest)
{
    glm::vec3 mini = glm::vec3(-1, -1, -1);
    glm::vec3 maxi = glm::vec3(1, 1, 1);
    OctTree tree(AABB(mini, maxi), 2);
    AABB firstQuadrant(mini, 0.8f*mini);
    tree.AddStaticObject(firstQuadrant);
    AABB testBox(0.9f*mini, 0.8f*mini);
    std::vector<AABB> region;
    tree.BoxesInSameRegion(testBox, region);
    BOOST_REQUIRE(region.size() == 1);
    AABB& box = region[0];
    BOOST_CHECK_CLOSE_FRACTION(box.Center().x, firstQuadrant.Center().x, 0.00001f);
    BOOST_CHECK_CLOSE_FRACTION(box.Center().y, firstQuadrant.Center().y, 0.00001f);
    BOOST_CHECK_CLOSE_FRACTION(box.Center().z, firstQuadrant.Center().z, 0.00001f);
    BOOST_CHECK_CLOSE_FRACTION(box.HalfSize().x, firstQuadrant.HalfSize().x, 0.00001f);
    BOOST_CHECK_CLOSE_FRACTION(box.HalfSize().y, firstQuadrant.HalfSize().y, 0.00001f);
    BOOST_CHECK_CLOSE_FRACTION(box.HalfSize().z, firstQuadrant.HalfSize().z, 0.00001f);
}

BOOST_AUTO_TEST_SUITE_END()

