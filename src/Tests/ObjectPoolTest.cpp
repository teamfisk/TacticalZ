#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;
#include "Engine/Core/ObjectPool.h"
#include <ctime>

struct S
{
    S() = default;
    S(int i, float ff) : k(i), f(ff) { }
    ~S() { }
    int k;
    float f;
};

//BOOST_GLOBAL_FIXTURE(S);
BOOST_AUTO_TEST_SUITE(memProtoTypeTestSuite)

BOOST_AUTO_TEST_CASE(testPool)
{
    ObjectPool<S> pool(32);//32 true/false values = 32 slots
    BOOST_CHECK(pool.empty() == true);

    const size_t size = 12;//12 platser i structen addresses, som håller en int, en float vardera
    S* addresses[size];
    addresses[0] = pool.New(7, 0.035f);
    //"Not empty after allocating one element."
    BOOST_CHECK(!pool.empty());

    //"Element created correctly with k==7"
    BOOST_CHECK(addresses[0]->k == 7);
    //"Element created correctly with f==0.035f"
    BOOST_CHECK_CLOSE_FRACTION(addresses[0]->f, 0.035f, 0.0001f);
    addresses[0]->k = 5;
    BOOST_CHECK(addresses[0]->k == 5);

    //"Empty after delete"
    pool.Delete(addresses[0]);
    BOOST_CHECK(pool.empty());

    addresses[0] = pool.New(7, 0.035f);
    addresses[1] = pool.New(5, 0.035f);
    pool.Delete(addresses[1]);
    BOOST_CHECK(!pool.empty());
    pool.Delete(addresses[0]);
    BOOST_CHECK(pool.empty());

    //INT32_MAX, FLT_MAX test
    addresses[0] = pool.New(INT32_MAX, FLT_MAX);
    BOOST_CHECK(!pool.empty());
    BOOST_CHECK(addresses[0]->k == INT32_MAX);
    BOOST_CHECK_CLOSE_FRACTION(addresses[0]->f, FLT_MAX, 0.0001f);
}
/*
BOOST_AUTO_TEST_CASE(testPoolArray)
{
ObjectPool<S> pool(32);
S* addresses;

//Add array size 5 to pool."
addresses = pool.NewArray(5);// <-> addresses = new S[5];
addresses[0] = S(12, 0.030f);
addresses[1] = S(13, 0.031f);
addresses[2] = S(14, 0.032f);
addresses[3] = S(15, 0.033f);
addresses[4] = S(16, 0.034f);

//"Not empty after allocating
BOOST_CHECK(!pool.empty());
//"Element created correctly with k==12"
BOOST_CHECK(addresses->k == 12);
//"Element created correctly with f==0.030f"
BOOST_CHECK_CLOSE_FRACTION(addresses->f, 0.030f, 0.0001f);

//add a few other structs so it becomes bigger than the original size (32),
//which means it must push back the rest of the values into a vector
S* test2, *test3, *test4, *test5;
test2 = pool.NewArray(5);// <-> test2 = new S[5];
test3 = pool.NewArray(40);//+40
test4 = pool.NewArray(40);//+40
test5 = pool.NewArray(40);//+40=120
BOOST_CHECK(pool.ExtraSize() == 120);
BOOST_CHECK(pool.PoolSize() == 10);
BOOST_CHECK(pool.size() == 120 + 10);

//testar "perfekt delete", dvs bryr mig inte om att testa att deleta bara 38 om storleken egentligen är 40 osv
pool.DeleteArray(test2, 5);//callar destructorn på test2 också
pool.DeleteArray(test3, 40);

pool.DeleteArray(addresses, 5);

//add / del array
S* another = pool.NewArray(64);
for (int i = 0; i < 64; ++i)
another[i] = S(i, 0.1f*i);
pool.DeleteArray(another, 64);
}
*/
BOOST_AUTO_TEST_CASE(testIterationNormal)
{
    //extra vector check
    S* test4, *test5;
    ObjectPool<S> pool(4);
    test4 = pool.New();
    test5 = pool.New();
    //Check so iterate over pool doesn't throw compile-time errors.
    for (auto &o : pool)
        o.k = 14;
    for (size_t i = 0; i < 1; ++i)
        BOOST_CHECK(test4[i].k == 14);
    for (size_t i = 0; i < 1; ++i)
        BOOST_CHECK(test5[i].k == 14);
}

BOOST_AUTO_TEST_CASE(testOutOfScopeDelete)
{
    //extra vector check
    S* test4, *test5;
    {
        ObjectPool<S> pool(4);
        test4 = pool.New();
        test5 = pool.New();
        //Check so iterate over pool doesn't throw compile-time errors.
        for (auto &o : pool)
            o.k = 14;
        for (size_t i = 0; i < 1; ++i)
            BOOST_CHECK(test4[i].k == 14);
        for (size_t i = 0; i < 1; ++i)
            BOOST_CHECK(test5[i].k == 14);
    }
    //pool goes out of scope here, and thus the test4 values become undefined (memory is killed at out of scope)
    BOOST_CHECK(test4[0].k != 14);
    BOOST_CHECK(test5[0].k != 15);
}

BOOST_AUTO_TEST_CASE(testIterationOneExtra)
{
    //extra vector check
    ObjectPool<S> pool(1);
    S* test4, *test5;
    test4 = pool.New();
    test5 = pool.New();
    //Check so iterate over pool doesn't throw compile-time errors.
    for (auto &o : pool)
        o.k = 14;
    for (size_t i = 0; i < 1; ++i)
        BOOST_CHECK(test4[i].k == 14);
    for (size_t i = 0; i < 1; ++i)
        BOOST_CHECK(test5[i].k == 14);
}

BOOST_AUTO_TEST_CASE(testIterationTwoExtra)
{
    //extra vector check
    ObjectPool<S> pool(1);
    S* test4, *test5;
    test4 = pool.New();
    test5 = pool.New();
    //Check so iterate over pool doesn't throw compile-time errors.
    for (auto &o : pool)
        o.k = 14;
    for (size_t i = 0; i < 1; ++i)
        BOOST_CHECK(test4[i].k == 14);
    for (size_t i = 0; i < 1; ++i)
        BOOST_CHECK(test5[i].k == 14);
}
/*
BOOST_AUTO_TEST_CASE(releaseModeTest_RandomAllocateDeallocate)
{
//run this in releasemode
struct I
{
I() = default;
I(size_t i, size_t ff) : k(i), f(ff) { }
~I() { }
size_t k;
size_t f;
};
srand((unsigned int)time(nullptr));

const size_t SIZE = 128;
ObjectPool<I> pool(SIZE);
I* addresses[SIZE];
std::vector<bool> allocated(SIZE, false);
std::vector<size_t> arrSizes(SIZE, 0);
size_t slotsAlloced = 0;
size_t superCount = 0;
size_t slot;
size_t i;

while (superCount++ < 1000) {
if (rand() % 2 == 0) {
i = 0;
//ta slumpmässig slot som inte är allokerad
do {
slot = (size_t)((SIZE - 1) * ((float)rand() / RAND_MAX));
} while (allocated[slot] && ++i < 512);

if (i < 512) {
arrSizes[slot] = 1 + (size_t)((24 - 1) * ((float)rand() / RAND_MAX));
addresses[slot] = pool.NewArray(arrSizes[slot]);
for (size_t a = 0; a < arrSizes[slot]; ++a)
addresses[slot][a] = I(slot, a);
allocated[slot] = true;
++slotsAlloced;
}
}
//Deallocate
else {
i = 0;
//ta slumpmässig slot som är allokerad
do {
slot = (size_t)((SIZE - 1) * ((float)rand() / RAND_MAX));
} while (!allocated[slot] && ++i < 512);

if (i < 512) {
pool.DeleteArray(addresses[slot], arrSizes[slot]);
arrSizes[slot] = 0;
allocated[slot] = false;
--slotsAlloced;
}
}
//Check content.
for (size_t a = 0; a < SIZE; ++a) {
if (allocated[a]) {
for (size_t e = 0; e < arrSizes[a]; ++e) {
BOOST_CHECK(!(addresses[a][e].k != a || addresses[a][e].f != e));
}
}
}
}
}
*/

BOOST_AUTO_TEST_CASE(testConstructors)
{
    //http://stackoverflow.com/questions/357929/is-it-important-to-unit-test-a-constructor
    //"If your constructor has, for example, an if (condition), you need to test both flows (true,false). 
    //If your constructor does some kind of job before setting. You should check the job is done"

    //testing the constructors with different T values and a small check so size is initialized to 0
    MemoryPool<int> memPoolI;
    BOOST_CHECK(memPoolI.empty());
    BOOST_CHECK(memPoolI.size() == 0);
    MemoryPool<float> memPoolF;
    BOOST_CHECK(memPoolF.empty());
    BOOST_CHECK(memPoolF.size() == 0);
    MemoryPool<double> memPoolD;
    BOOST_CHECK(memPoolD.empty());
    BOOST_CHECK(memPoolD.size() == 0);
    MemoryPool<S> memPoolS;
    BOOST_CHECK(memPoolS.empty());
    BOOST_CHECK(memPoolS.size() == 0);

    ObjectPool<int> objPoolI(64);
    BOOST_CHECK(objPoolI.empty());
    BOOST_CHECK(objPoolI.size() == 0);
    ObjectPool<float> objPoolF(32);
    BOOST_CHECK(objPoolF.empty());
    BOOST_CHECK(objPoolF.size() == 0);
    ObjectPool<double> objPoolD(16);
    BOOST_CHECK(objPoolD.empty());
    BOOST_CHECK(objPoolD.size() == 0);
    ObjectPool<S> objPoolS(128);
    BOOST_CHECK(objPoolS.empty());
    BOOST_CHECK(objPoolS.size() == 0);
}

BOOST_AUTO_TEST_CASE(testOperators)
{
    ObjectPool<S> pool(100);
    //	S* s[12] = pool.NewArray(12);
    S* s[12];
    s[0] = pool.New();
    s[11] = pool.New();

    s[0]->k = 2;
    s[11]->k = 3;
    //testing operators: ++i,!=
    auto& iter = pool.begin();
    for (iter; iter != pool.end(); ++iter) {
        //testing operators:*,==
        auto dereferencedIterator = *iter;
        if (iter == pool.begin()) {
            BOOST_CHECK(dereferencedIterator.k == 2);
        }
        if (iter == pool.end()) {
            BOOST_CHECK(dereferencedIterator.k == 3);
        }
        //testing operators:->
        iter->k += 2;
    }
    BOOST_CHECK(s[0]->k == 4);
    BOOST_CHECK(s[11]->k == 5);
    BOOST_CHECK(iter == pool.end());

    //testing operators:i++
    s[0]->k = 2;
    s[11]->k = 2;
    for (auto& iter = pool.begin(); iter != pool.end(); iter++)
        iter->k += 2;
    BOOST_CHECK(s[0]->k == 4);
    BOOST_CHECK(s[11]->k == 4);
}
/*
BOOST_AUTO_TEST_CASE(testBranchFree)
{
//testing Free , which is the only untested
//via delete/deletearray

//1. no extra memory delete
ObjectPool<S> pool(32);//32 true/false values = 32 slots
S* addresses[12];
addresses[0] = pool.New(7, 0.035f);
pool.Delete(addresses[0]);
BOOST_CHECK(pool.empty());

//1b. no extra memory deleteArray
ObjectPool<S> pool1b(32);//32 true/false values = 32 slots
S* test1b;
test1b = pool1b.NewArray(5);// <-> test2 = new S[5];
BOOST_CHECK(pool1b.size() == 5);
pool1b.DeleteArray(test1b, 5);//callar destructorn på test2 också
BOOST_CHECK(pool1b.empty());

//2. extra memory delete
ObjectPool<S> pool2(2);
S* addresses2[12];
addresses2[0] = pool2.New(7, 0.035f);
addresses2[1] = pool2.New(7, 0.035f);
addresses2[2] = pool2.New(7, 0.035f);
addresses2[3] = pool2.New(7, 0.035f);
addresses2[4] = pool2.New(7, 0.035f);
BOOST_CHECK(pool2.size() == 5);
pool2.Delete(addresses2[0]);
BOOST_CHECK(pool2.size() == 4);
pool2.Delete(addresses2[1]);
BOOST_CHECK(pool2.size() == 3);
pool2.Delete(addresses2[2]);
BOOST_CHECK(pool2.size() == 2);
pool2.Delete(addresses2[3]);
BOOST_CHECK(pool2.size() == 1);
pool2.Delete(addresses2[4]);
BOOST_CHECK(pool2.empty());
//reverse delete
addresses2[0] = pool2.New(7, 0.035f);
addresses2[1] = pool2.New(7, 0.035f);
addresses2[2] = pool2.New(7, 0.035f);
addresses2[3] = pool2.New(7, 0.035f);
addresses2[4] = pool2.New(7, 0.035f);
BOOST_CHECK(pool2.size() == 5);
pool2.Delete(addresses2[4]);
BOOST_CHECK(pool2.size() == 4);
pool2.Delete(addresses2[3]);
BOOST_CHECK(pool2.size() == 3);
pool2.Delete(addresses2[2]);
BOOST_CHECK(pool2.size() == 2);
pool2.Delete(addresses2[1]);
BOOST_CHECK(pool2.size() == 1);
pool2.Delete(addresses2[0]);
BOOST_CHECK(pool2.empty());

//2b. extra memory deleteArray
ObjectPool<S> pool2b(32);//32 true/false values = 32 slots
S* test2b,*test2bb;
test2b = pool2b.NewArray(5);// <-> test2 = new S[5];
BOOST_CHECK(pool2b.size() == 5);
test2bb = pool2b.NewArray(40);// <-> test2 = new S[5];
BOOST_CHECK(pool2b.size() == 45);
pool2b.DeleteArray(test2b, 5);//callar destructorn på test2 också
BOOST_CHECK(pool2b.size() == 40);
pool2b.DeleteArray(test2bb, 40);//callar destructorn på test2 också
BOOST_CHECK(pool2b.empty());
}
*/
BOOST_AUTO_TEST_CASE(testBranchAllocate)
{
    //1 slot else many slots
    //see testBranchFree

    //out of mem vs not out of mem allocate
    //see testBranchFree
}
BOOST_AUTO_TEST_CASE(testEdgeCase)
{
    //test with a very small pool
    ObjectPool<S> pool(1);
    BOOST_CHECK(pool.empty());
    S* test4;
    test4 = pool.New(7, 0.035f);
    BOOST_CHECK(!pool.empty());
    BOOST_CHECK(test4->k == 7);
    BOOST_CHECK_CLOSE_FRACTION(test4->f, 0.035f, 0.0001f);

    //test with a very small pool and array,  iterating
    ObjectPool<S> poolA(1);
    S* test5;
    test5 = poolA.New();
    //Check so iterate over pool doesn't throw compile-time errors.
    for (auto &o : poolA)
        o.k = 14;
    for (size_t i = 0; i < 1; ++i)
        BOOST_CHECK(test5[i].k == 14);
}

BOOST_AUTO_TEST_CASE(testBadlyAlignedData)
{
    //small test with non-aligned data 4+1bytes
    struct S
    {
        S() = default;
        S(float f, char c) : m_f(f), m_c(c) { }
        ~S() { }
        float m_f;
        char m_c;
    };
    MemoryPool<S> memPoolS;
    BOOST_CHECK(memPoolS.empty());
    BOOST_CHECK(memPoolS.size() == 0);
    ObjectPool<S> objPoolS(64);
    BOOST_CHECK(objPoolS.empty());
    BOOST_CHECK(objPoolS.size() == 0);
    S* test4;
    test4 = objPoolS.New();
    //Check so iterate over pool doesn't throw compile-time errors.
    for (auto &o : objPoolS) {
        o.m_c = 'v';
        o.m_f = 0.15534543f;
    }
    for (size_t i = 0; i < 1; ++i) {
        BOOST_CHECK(test4[i].m_c == 'v');
        BOOST_CHECK_CLOSE_FRACTION(test4[i].m_f, 0.15534543f, 0.0001f);
    }
}
BOOST_AUTO_TEST_CASE(testBadlyAlignedData2)
{
    //small test with non-aligned data 1+1+1bytes
    struct S
    {
        S() = default;
        S(char c, char c2, char c3) : m_c(c), m_c2(c2), m_c3(c3) { }
        ~S() { }
        char m_c;
        char m_c2;
        char m_c3;
    };
    MemoryPool<S> memPoolS;
    BOOST_CHECK(memPoolS.empty());
    BOOST_CHECK(memPoolS.size() == 0);
    ObjectPool<S> objPoolS(64);
    BOOST_CHECK(objPoolS.empty());
    BOOST_CHECK(objPoolS.size() == 0);
    S* test4;
    test4 = objPoolS.New();
    //Check so iterate over pool doesn't throw compile-time errors.
    for (auto &o : objPoolS) {
        o.m_c = 'v';
        o.m_c2 = 'w';
        o.m_c3 = 'x';
    }
    for (size_t i = 0; i < 1; ++i) {
        BOOST_CHECK(test4[i].m_c == 'v');
        BOOST_CHECK(test4[i].m_c2 == 'w');
        BOOST_CHECK(test4[i].m_c3 == 'x');
    }
}

BOOST_AUTO_TEST_CASE(testWrongData)
{
}
BOOST_AUTO_TEST_CASE(testFillDeleteFillAgain) {
    //already done in BOOST_AUTO_TEST_CASE(releaseModeTest_RandomAllocateDeallocate)

}

BOOST_AUTO_TEST_SUITE_END()
