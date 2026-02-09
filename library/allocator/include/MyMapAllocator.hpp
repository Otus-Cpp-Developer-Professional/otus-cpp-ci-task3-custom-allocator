#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

#include "detail/Arena.hpp"

namespace my_allocator::policy
{
    // Allocation policy: fixed number of elements (no expansion)
    struct FixedCapacity {};

    // Allocation policy: expandable arena
    struct ExpandableCapacity {};
}



/**
 * @brief STL-compatible allocator with optional fixed or expandable capacity
 *
 * MyMapAllocator is a stateful allocator that allocates memory from
 * an internal byte-based arena. The allocator itself operates in terms
 * of elements, as required by the STL allocator interface.
 *
 * Two allocation modes are supported:
 *
 * 1. FixedCapacity (default)
 *    - allocator is parameterized by the maximum number of elements
 *    - allocation beyond this limit results in std::bad_alloc
 *    - arena does NOT grow
 *
 * 2. ExpandableCapacity
 *    - allocator may expand the arena when capacity is exceeded
 *    - element limit is relaxed
 *
 * The allocator does NOT support individual deallocation.
 * All memory is released when the allocator (and its arena) is destroyed.
 *
 * @tparam T Value type to allocate
 * @tparam CapacityPolicy Allocation growth policy
 *
 * @note This allocator is not thread-safe
 * @note deallocate() is intentionally a no-op
 */
template<
        typename T,
        typename CapacityPolicy = my_allocator::policy::FixedCapacity
>
class MyMapAllocator
{
public:
    using value_type = T;

private:
    using Arena = my_allocator::detail::Arena;

    std::shared_ptr<Arena> arena_;

    std::size_t max_elements_ = 0;     // logical capacity in elements
    std::size_t allocated_    = 0;     // allocated elements count

public:
    /// Default constructor is disabled: capacity must be specified
    MyMapAllocator() = delete;

    /**
     * @brief Constructs allocator with fixed element capacity
     *
     * This constructor is intended for FixedCapacity mode.
     * The arena size is computed as max_elements * sizeof(T).
     *
     * @param max_elements Maximum number of elements that can be allocated
     */
    explicit MyMapAllocator(std::size_t max_elements)
            : max_elements_(max_elements)
    {
        const std::size_t arena_bytes = max_elements * sizeof(T);
        arena_ = std::make_shared<Arena>(arena_bytes);
    }

    /**
     * @brief Copy constructor from allocator of another value type
     *
     * Required for STL allocator compatibility (rebind).
     * Shares the same underlying arena and allocation state.
     */

    template<typename U>
    explicit MyMapAllocator(const MyMapAllocator<U, CapacityPolicy>& other) noexcept
            : arena_(other.arena_)
            , max_elements_(other.max_elements_)
            , allocated_(other.allocated_)
    {}

    /**
     * @brief Allocates memory for n elements of type T
     *
     * @param n Number of elements
     * @return Pointer to allocated memory
     *
     * @throws std::bad_alloc if capacity is exceeded (FixedCapacity)
     */
    T* allocate(std::size_t n)
    {
        // Enforce element limit in fixed-capacity mode
        using namespace my_allocator::policy;

        if constexpr (std::is_same_v<CapacityPolicy, FixedCapacity>)
        {
            if (allocated_ + n > max_elements_)
                throw std::bad_alloc{};
        }

        void* ptr = arena_->allocate_bytes(
                n * sizeof(T),
                alignof(T)
        );

        allocated_ += n;
        return static_cast<T*>(ptr);
    }

    /**
     * @brief Deallocation function (no-op)
     *
     * Individual deallocation is intentionally not supported.
     * Memory is reclaimed only when the allocator is destroyed.
     */
    void deallocate(T*, std::size_t) noexcept
    {
        // no-op
    }

    // Required for allocator equality comparison in STL
    template<typename U, typename P>
    friend class MyMapAllocator;
};
