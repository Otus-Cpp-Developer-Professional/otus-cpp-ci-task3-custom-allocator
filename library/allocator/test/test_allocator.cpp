#define BOOST_TEST_MODULE my_map_allocator_tests
#include <boost/test/included/unit_test.hpp>

#include <map>
#include <type_traits>

#include "MyMapAllocator.hpp"


BOOST_AUTO_TEST_CASE(allocate_single_int)
{
    MyMapAllocator<int> alloc(1024);

    int* p = alloc.allocate(1);
    BOOST_REQUIRE(p != nullptr);

    *p = 42;
    BOOST_CHECK_EQUAL(*p, 42);
}


BOOST_AUTO_TEST_CASE(allocate_array)
{
    MyMapAllocator<int> alloc(1024);

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

    MyMapAllocator<BigAligned> alloc(1024);

    BigAligned* p = alloc.allocate(1);
    auto addr = reinterpret_cast<std::uintptr_t>(p);

    BOOST_CHECK(addr % alignof(BigAligned) == 0);
}


BOOST_AUTO_TEST_CASE(map_with_custom_allocator)
{
    using Map =
            std::map<
                    int,
                    int,
                    std::less<int>,
                    MyMapAllocator<std::pair<const int, int>>
            >;

    Map m{ MyMapAllocator<std::pair<const int, int>>(4096) };

    m.emplace(1, 10);
    m.emplace(2, 20);
    m.emplace(3, 30);

    BOOST_CHECK_EQUAL(m[1], 10);
    BOOST_CHECK_EQUAL(m[2], 20);
    BOOST_CHECK_EQUAL(m[3], 30);
}


BOOST_AUTO_TEST_CASE(allocator_is_copyable)
{
    MyMapAllocator<int> a1(1024);
    MyMapAllocator<int> a2(a1);

    int* p = a2.allocate(1);
    BOOST_REQUIRE(p != nullptr);

    *p = 42;
    BOOST_CHECK_EQUAL(*p, 42);
}

