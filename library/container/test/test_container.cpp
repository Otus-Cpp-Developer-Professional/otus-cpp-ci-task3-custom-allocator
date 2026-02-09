#define BOOST_TEST_MODULE mycontainer_tests
#include <boost/test/included/unit_test.hpp>

#include <vector>
#include <MyContainer.hpp>

BOOST_AUTO_TEST_SUITE(mycontainer_basic)


    BOOST_AUTO_TEST_CASE(default_constructed_is_empty)
    {
        MyContainer<int> c;

        BOOST_CHECK(c.empty());
        BOOST_CHECK_EQUAL(c.size(), 0u);
    }


    BOOST_AUTO_TEST_CASE(push_front_single)
    {
        MyContainer<int> c;

        c.push_front(42);

        BOOST_CHECK(!c.empty());
        BOOST_CHECK_EQUAL(c.size(), 1u);
        BOOST_CHECK_EQUAL(*c.begin(), 42);
    }

    BOOST_AUTO_TEST_CASE(push_front_order)
    {
        MyContainer<int> c;

        c.push_front(1);
        c.push_front(2);
        c.push_front(3);

        std::vector<int> v;
        for (int x : c)
            v.push_back(x);

        std::vector<int> expected{3, 2, 1};

        BOOST_CHECK_EQUAL_COLLECTIONS(
                v.begin(), v.end(),
                expected.begin(), expected.end()
        );

    }


    BOOST_AUTO_TEST_CASE(push_back_single)
    {
        MyContainer<int> c;

        c.push_back(7);

        BOOST_CHECK_EQUAL(c.size(), 1u);
        BOOST_CHECK_EQUAL(*c.begin(), 7);
    }

    BOOST_AUTO_TEST_CASE(push_back_order)
    {
        MyContainer<int> c;

        c.push_back(1);
        c.push_back(2);
        c.push_back(3);

        std::vector<int> v;
        for (int x : c)
            v.push_back(x);

        std::vector<int> expected{1, 2, 3};

        BOOST_CHECK_EQUAL_COLLECTIONS(
                v.begin(), v.end(),
                expected.begin(), expected.end()
        );

    }


    BOOST_AUTO_TEST_CASE(mixed_push)
    {
        MyContainer<int> c;

        c.push_back(2);
        c.push_front(1);
        c.push_back(3);

        std::vector<int> v;
        for (int x : c)
            v.push_back(x);

        std::vector<int> expected{1, 2, 3};

        BOOST_CHECK_EQUAL_COLLECTIONS(
                v.begin(), v.end(),
                expected.begin(), expected.end()
        );

    }


    BOOST_AUTO_TEST_CASE(pop_front_basic)
    {
        MyContainer<int> c;

        c.push_back(1);
        c.push_back(2);
        c.push_back(3);

        c.pop_front();

        BOOST_CHECK_EQUAL(c.size(), 2u);
        BOOST_CHECK_EQUAL(*c.begin(), 2);
    }

    BOOST_AUTO_TEST_CASE(pop_front_until_empty)
    {
        MyContainer<int> c;

        c.push_back(1);
        c.pop_front();

        BOOST_CHECK(c.empty());
        BOOST_CHECK_EQUAL(c.size(), 0u);
    }

    BOOST_AUTO_TEST_CASE(pop_front_on_empty_is_safe)
    {
        MyContainer<int> c;

        c.pop_front(); // must not crash
        BOOST_CHECK(c.empty());
    }


    BOOST_AUTO_TEST_CASE(clear_empties_container)
    {
        MyContainer<int> c;

        for (int i = 0; i < 10; ++i)
            c.push_back(i);

        c.clear();

        BOOST_CHECK(c.empty());
        BOOST_CHECK_EQUAL(c.size(), 0u);
        BOOST_CHECK(c.begin() == c.end());
    }


    BOOST_AUTO_TEST_CASE(iterator_traversal)
    {
        MyContainer<int> c;

        for (int i = 1; i <= 5; ++i)
            c.push_back(i);

        int sum = 0;
        for (auto it = c.begin(); it != c.end(); ++it)
            sum += *it;

        BOOST_CHECK_EQUAL(sum, 15); // 1+2+3+4+5
    }

    BOOST_AUTO_TEST_CASE(iterator_postincrement)
    {
        MyContainer<int> c;
        c.push_back(1);
        c.push_back(2);

        auto it = c.begin();
        BOOST_CHECK_EQUAL(*it++, 1);
        BOOST_CHECK_EQUAL(*it, 2);
    }

    BOOST_AUTO_TEST_CASE(allocator_compatibility_smoke)
    {
        MyContainer<int, std::allocator<int>> c;

        for (int i = 0; i < 5; ++i)
            c.push_back(i);

        BOOST_CHECK_EQUAL(c.size(), 5u);
    }


BOOST_AUTO_TEST_SUITE_END()
