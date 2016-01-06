#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;
#include <stdlib.h>//srand

//#define private public//HACK! Needed for white box testing
//#include "Engine/Core/OctTree.h"
//#include "OldOctTree.h"
//friend class and refactoringIntoNewClass is some extra work and needs to be updated when the original class is updated, and can contain bugs that
//isnt in the original class
//Reflection-inspection seems to be only available for C#
//http://stackoverflow.com/questions/6778496/how-to-do-unit-testing-on-private-members-and-methods-of-c-classes
//http://stackoverflow.com/questions/3676664/unit-testing-of-private-methods

#include "OctTreeTestGameClass.h"

#define private public//HACK! Needed for white box testing
#include <Engine\Core\OctTree.h>
//else we would have to "open up" the octTree class more with get/sets, public methods, etc. which is not good encapsulation-wise

BOOST_AUTO_TEST_SUITE(octTreeTestsA)

BOOST_AUTO_TEST_CASE(octTreeTest)
{
    //white box testing
    //http://softwaretestingfundamentals.com/differences-between-black-box-testing-and-white-box-testing/
    //http://technologyconversations.com/2013/12/11/black-box-vs-white-box-testing/

    //simple AABB constructor check
    auto minCorner = glm::vec3(0.0f, 0.0f, 0.0f);
    auto maxCorner = glm::vec3(1.0f, 1.0f, 1.0f);
    auto someAABB = AABB(minCorner, maxCorner);
    BOOST_CHECK(someAABB.MinCorner() == minCorner);
    BOOST_CHECK(someAABB.MaxCorner() == maxCorner);
    BOOST_CHECK(someAABB.Center() == 0.5f * (minCorner + maxCorner));

    //simple OctTree constructor check
    //OctTree someOctTree(someAABB, 5);
    //BOOST_CHECK(someOctTree.m_Children[0] != nullptr);

    //simple destructor check in the end, just look for memleaks, then it didnt clear the AABB structure
}

BOOST_AUTO_TEST_CASE(octTreeTest2)
{
    //octtree ritningen osv
    Game game(0, nullptr);
    while (game.Running()) {
        game.Tick();
    }
}

BOOST_AUTO_TEST_SUITE_END()
