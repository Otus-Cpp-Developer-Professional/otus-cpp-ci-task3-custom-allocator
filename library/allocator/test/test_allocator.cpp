#define BOOST_TEST_MODULE my_map_allocator_tests
#include <boost/test/included/unit_test.hpp>

#include <map>
#include <type_traits>
#include <cstdint>
#include <forward_list>

#include "MyMapAllocator.hpp"

namespace policy = my_allocator::policy;



// ============================================================
// Expandable basic allocation
// ============================================================

BOOST_AUTO_TEST_CASE(allocate_single_int_expandable)
{
    using Alloc = MyMapAllocator<int, policy::Expandable<32>>;
    Alloc alloc;

    int* p = alloc.allocate(1);
    BOOST_REQUIRE(p != nullptr);

    *p = 42;
    BOOST_CHECK_EQUAL(*p, 42);
}

BOOST_AUTO_TEST_CASE(allocate_array_expandable)
{
    using Alloc = MyMapAllocator<int, policy::Expandable<32>>;
    Alloc alloc;

    constexpr std::size_t N = 10;
    int* p = alloc.allocate(N);

    for (std::size_t i = 0; i < N; ++i)
        p[i] = static_cast<int>(i);

    for (std::size_t i = 0; i < N; ++i)
        BOOST_CHECK_EQUAL(p[i], i);
}



// ============================================================
// Alignment
// ============================================================

BOOST_AUTO_TEST_CASE(alignment_check)
{
    struct alignas(32) BigAligned {
        std::uint64_t data[4];
    };

    using Alloc = MyMapAllocator<BigAligned, policy::Expandable<8>>;
    Alloc alloc;

    BigAligned* p = alloc.allocate(1);
    auto addr = reinterpret_cast<std::uintptr_t>(p);

    BOOST_CHECK(addr % alignof(BigAligned) == 0);
}



// ============================================================
// Fixed capacity behavior
// ============================================================

BOOST_AUTO_TEST_CASE(fixed_capacity_throws_bad_alloc)
{
    using Alloc = MyMapAllocator<int, policy::Fixed<2>>;
    Alloc alloc;

    int* a = alloc.allocate(1);
    int* b = alloc.allocate(1);

    BOOST_REQUIRE(a != nullptr);
    BOOST_REQUIRE(b != nullptr);

    BOOST_CHECK_THROW(
            alloc.allocate(1),
            std::bad_alloc
    );
}



// ============================================================
// Expandable grows beyond initial
// ============================================================

BOOST_AUTO_TEST_CASE(expandable_allocator_grows)
{
    using Alloc = MyMapAllocator<int, policy::Expandable<1>>;
    Alloc alloc;

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



// ============================================================
// Shared state (fixed policy)
// ============================================================

BOOST_AUTO_TEST_CASE(copy_constructor_shares_state_fixed)
{
    using Alloc = MyMapAllocator<int, policy::Fixed<2>>;

    Alloc alloc1;
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
    using Alloc = MyMapAllocator<int, policy::Fixed<2>>;

    Alloc alloc1;
    Alloc alloc2;

    alloc2.allocate(1);
    alloc2.allocate(1);

    alloc1 = alloc2;

    BOOST_CHECK_THROW(
            alloc1.allocate(1),
            std::bad_alloc
    );
}



// ============================================================
// std::map integration (fixed)
// ============================================================

BOOST_AUTO_TEST_CASE(map_with_custom_allocator_fixed)
{
    using Alloc =
            MyMapAllocator<
                    std::pair<const int, int>,
                    policy::Fixed<16>
            >;

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



// ============================================================
// std::map integration (expandable)
// ============================================================

BOOST_AUTO_TEST_CASE(map_with_custom_allocator_expandable)
{
    using Alloc =
            MyMapAllocator<
                    std::pair<const int, int>,
                    policy::Expandable<2>
            >;

    using Map =
            std::map<int, int, std::less<int>, Alloc>;

    Map m;

    for (int i = 0; i < 10; ++i)
        m.emplace(i, i * 10);

    for (int i = 0; i < 10; ++i)
        BOOST_CHECK_EQUAL(m[i], i * 10);
}
