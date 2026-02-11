#define BOOST_TEST_MODULE my_map_allocator_tests
#include <boost/test/included/unit_test.hpp>

#include <map>
#include <type_traits>
#include <cstdint>
#include <forward_list>

#include "MyMapAllocator.hpp"

using my_allocator::ExpandableArenaInitialCapacity;



BOOST_AUTO_TEST_CASE(allocate_single_int_expandable)
{
    MyMapAllocator<int> alloc(ExpandableArenaInitialCapacity{32});

    int* p = alloc.allocate(1);
    BOOST_REQUIRE(p != nullptr);

    *p = 42;
    BOOST_CHECK_EQUAL(*p, 42);
}

BOOST_AUTO_TEST_CASE(allocate_array_expandable)
{
    MyMapAllocator<int> alloc(ExpandableArenaInitialCapacity{32});

    constexpr std::size_t N = 10;
    int* p = alloc.allocate(N);

    for (std::size_t i = 0; i < N; ++i)
        p[i] = static_cast<int>(i);

    for (std::size_t i = 0; i < N; ++i)
        BOOST_CHECK_EQUAL(p[i], i);
}



BOOST_AUTO_TEST_CASE(alignment_check)
{
    struct alignas(32) BigAligned {
        std::uint64_t data[4];
    };

    MyMapAllocator<BigAligned> alloc(
            ExpandableArenaInitialCapacity{8}
    );

    BigAligned* p = alloc.allocate(1);
    auto addr = reinterpret_cast<std::uintptr_t>(p);

    BOOST_CHECK(addr % alignof(BigAligned) == 0);
}



BOOST_AUTO_TEST_CASE(fixed_capacity_throws_bad_alloc)
{
    MyMapAllocator<int, 2> alloc;

    int* a = alloc.allocate(1);
    int* b = alloc.allocate(1);

    BOOST_REQUIRE(a != nullptr);
    BOOST_REQUIRE(b != nullptr);

    BOOST_CHECK_THROW(
            alloc.allocate(1),
            std::bad_alloc
    );
}



BOOST_AUTO_TEST_CASE(expandable_allocator_grows)
{
    MyMapAllocator<int> alloc(
            ExpandableArenaInitialCapacity{1}
    );

    int* a = alloc.allocate(1);
    int* b = nullptr;

    BOOST_REQUIRE(a != nullptr);

    BOOST_CHECK_NO_THROW(
            b = alloc.allocate(1)
    );

    BOOST_REQUIRE(b != nullptr);

    *a = 1;
    *b = 2;

    BOOST_CHECK_EQUAL(*a, 1);
    BOOST_CHECK_EQUAL(*b, 2);
}



BOOST_AUTO_TEST_CASE(copy_constructor_shares_state_fixed)
{
    MyMapAllocator<int, 2> alloc1;
    auto alloc2 = alloc1;

    alloc1.allocate(1);
    alloc2.allocate(1);

    BOOST_CHECK_THROW(
            alloc1.allocate(1),
            std::bad_alloc
    );

    BOOST_CHECK_THROW(
            alloc2.allocate(1),
            std::bad_alloc
    );
}

BOOST_AUTO_TEST_CASE(copy_assignment_shares_state_fixed)
{
    MyMapAllocator<int, 2> alloc1;
    MyMapAllocator<int, 2> alloc2;

    alloc2.allocate(1);
    alloc2.allocate(1);

    alloc1 = alloc2;

    BOOST_CHECK_THROW(
            alloc1.allocate(1),
            std::bad_alloc
    );
}



BOOST_AUTO_TEST_CASE(map_with_custom_allocator_fixed)
{
    using Alloc =
            MyMapAllocator<std::pair<const int, int>, 16>;

    using Map =
            std::map<int, int, std::less<int>, Alloc>;

    Map m;

    m.emplace(1, 10);
    m.emplace(2, 20);
    m.emplace(3, 30);

    BOOST_CHECK_EQUAL(m[1], 10);
    BOOST_CHECK_EQUAL(m[2], 20);
    BOOST_CHECK_EQUAL(m[3], 30);
}

BOOST_AUTO_TEST_CASE(vector_shared_budget_release_after_single_destroy)
{
    using Alloc = MyMapAllocator<int, 5>;
    using List   = std::forward_list<int, Alloc>;

    Alloc alloc;

    List l2(alloc); //1 allocate

    {
        List l1(alloc);

        l1.push_front(1);
        l2.push_front(20);


        BOOST_CHECK_NO_THROW(l2.push_front(2));

        BOOST_CHECK_THROW(
                l2.push_front(3),
                std::bad_alloc
        );
    } //l1 dead - 2 deallocate

    BOOST_CHECK_NO_THROW(l2.push_front(40));
    BOOST_CHECK_NO_THROW(l2.push_front(50));

    BOOST_CHECK_THROW(l2.push_front(60), std::bad_alloc);
}



