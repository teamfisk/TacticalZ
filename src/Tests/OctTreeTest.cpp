#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;
#include <stdlib.h>//srand
#include "Engine/Core/OctTree.h"
#include "Engine/Core/Ray.h"
#include "OldOctTree.h"

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

const int LEVEL_BOUNDS = 500;
const int MAXSIZE = 50;
const int BOXES = 400;
const int NUM_DYNAMICS = 0;
const int NUM_STATICS = BOXES - NUM_DYNAMICS;
const int SEED = 6548;
const int TEST_FRAMES = 300;
const int NUM_FUNCTION_LOOPS = 25;
const int TESTS = 0;                //10

template<typename Tree>
void RegionTest(Tree& tree)
{
    AABB aabb;
    aabb.CreateFromCenter(glm::vec3(rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS),
        glm::vec3(rand() % MAXSIZE, rand() % MAXSIZE, rand() % MAXSIZE));
    std::vector<AABB> outVec;
    tree.BoxesInSameRegion(aabb, outVec);
}

template<typename Tree>
void RayTest(Tree& tree)
{
    Tree::Output data;
    glm::vec3 rayStart = glm::vec3(rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS);
    glm::vec3 rayEnd = glm::vec3(rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS);
    tree.RayCollides({ rayStart , glm::normalize(rayEnd - rayStart) }, data);
}

template<typename Tree>
void BoxTest(Tree& tree)
{
    AABB outBox;
    AABB aabb;
    aabb.CreateFromCenter(glm::vec3(rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS),
        glm::vec3(rand() % MAXSIZE, rand() % MAXSIZE, rand() % MAXSIZE));
    tree.BoxCollides(aabb, outBox);
}

template<typename Tree>
void NopTest(Tree& tree)
{

}

template<typename Tree, typename TestFunction>
void TestLoop(TestFunction xTest)
{
    srand(SEED);
    glm::vec3 mini = glm::vec3(0, 0, 0);
    glm::vec3 maxi = glm::vec3(LEVEL_BOUNDS, LEVEL_BOUNDS, LEVEL_BOUNDS);
    Tree tree(AABB(mini, maxi), 3);
    AABB aabb;
    glm::vec3 center;
    glm::vec3 size;
    for (int t = 0; t < TESTS; ++t) {
        for (int i = 0; i < NUM_STATICS; ++i) {
            center = glm::vec3(rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS);
            size = glm::vec3(rand() % MAXSIZE, rand() % MAXSIZE, rand() % MAXSIZE);
            aabb.CreateFromCenter(center, size);
            tree.AddStaticObject(aabb);
        }
        for (int fr = 0; fr < TEST_FRAMES; ++fr) {
            for (int i = 0; i < NUM_DYNAMICS; ++i) {
                center = glm::vec3(rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS, rand() % LEVEL_BOUNDS);
                size = glm::vec3(rand() % MAXSIZE, rand() % MAXSIZE, rand() % MAXSIZE);
                aabb.CreateFromCenter(center, size);
                tree.AddDynamicObject(aabb);
            }

            for (int fl = 0; fl < NUM_FUNCTION_LOOPS; ++fl) {
                xTest(tree);
            }

            tree.ClearDynamicObjects();
        }
        tree.ClearObjects();
    }
}

BOOST_AUTO_TEST_CASE(octRegionPerfTestWithDuplicates)
{
    TestLoop<Old::OctTree>(RegionTest<Old::OctTree>);
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(octRegionPerfTestNoDuplicates)
{
    TestLoop<OctTree>(RegionTest<OctTree>);
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(octBoxPerfTestWithDuplicates)
{
    TestLoop<Old::OctTree>(BoxTest<Old::OctTree>);
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(octBoxPerfTestNoDuplicates)
{
    TestLoop<OctTree>(BoxTest<OctTree>);
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(octRayPerfTestWithDuplicates)
{
    TestLoop<Old::OctTree>(RayTest<Old::OctTree>);
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(octRayPerfTestNoDuplicates)
{
    TestLoop<OctTree>(RayTest<OctTree>);
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(octNopPerfTestWithDuplicates)
{
    TestLoop<Old::OctTree>(NopTest<Old::OctTree>);
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(octNopPerfTestNoDuplicates)
{
    TestLoop<OctTree>(NopTest<OctTree>);
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()

