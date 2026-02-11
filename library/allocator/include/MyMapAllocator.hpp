#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

#include "detail/Arena.hpp"

namespace my_allocator {

    namespace detail
    {
        /**
         * @brief Internal shared state of the allocator.
         *
         * This structure stores logical allocation statistics:
         *
         * - max_elements_  – maximum number of elements allowed (fixed mode)
         * - allocated_     – number of currently allocated elements
         *
         * The state is shared between allocator copies via std::shared_ptr,
         * meaning copies of the allocator share the same logical budget.
         */
        struct AllocatorState {

            std::size_t max_elements_ = 0;
            std::size_t allocated_ = 0;

            AllocatorState() = default;
            explicit AllocatorState(std::size_t max_elements)
                    : max_elements_(max_elements) {}
        };
    }


    /**
     * @brief Tag type for expandable allocator mode.
     *
     * Used only when MaxElements == 0.
     * Allows user to specify initial arena size in number of elements.
     */
    struct ExpandableArenaInitialCapacity
    {
        std::size_t value;
    };
}


/**
 * @brief STL-compatible arena-based allocator.
 *
 * This allocator provides two operation modes depending on MaxElements:
 *
 * 1. Fixed capacity mode (MaxElements > 0)
 *    - Maximum number of elements is known at compile time.
 *    - Allocation beyond this limit throws std::bad_alloc.
 *    - The arena size is exactly MaxElements * sizeof(T).
 *
 * 2. Expandable mode (MaxElements == 0)
 *    - No element limit is enforced.
 *    - The arena may grow when capacity is exceeded.
 *    - Initial capacity is provided via ExpandableArenaInitialCapacity.
 *
 * Copies of the allocator share:
 *  - underlying Arena
 *  - allocation state
 *
 * This means the logical allocation budget is shared across containers
 * that use copies of the allocator.
 *
 * Deallocation does not release physical memory inside the arena.
 * Memory is reclaimed only when the last allocator copy is destroyed.
 *
 * @tparam T           Value type
 * @tparam MaxElements Maximum number of elements (0 = expandable mode)
 *
 * @note Not thread-safe.
 */
template<
        typename T,
        std::size_t MaxElements = 0
>
class MyMapAllocator
{

public:

    /**
     * @brief Rebind support required by STL containers.
     *
     * Allows containers to rebind allocator to internal node types.
     */
    template<typename U>
    struct rebind
    {
        using other = MyMapAllocator<U, MaxElements>;
    };

    using value_type = T;

    /**
     * @brief Propagation traits.
     *
     * Allocator is stateful and shared. Therefore:
     * - copy assignment propagates allocator
     * - move assignment propagates allocator
     * - swap propagates allocator
     */
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap            = std::true_type;

    /**
     * @brief Allocators are not always equal.
     *
     * Equality depends on shared arena identity.
     */
    using is_always_equal = std::false_type;

private:

    using Arena = my_allocator::detail::Arena;
    using AllocatorState = my_allocator::detail::AllocatorState;

    /// Shared allocation accounting
    std::shared_ptr<AllocatorState> state_;

    /// Shared underlying memory arena
    std::shared_ptr<Arena> arena_;

public:

    /**
     * @brief Constructor for fixed-capacity mode.
     *
     * Enabled only when MaxElements > 0.
     *
     * Arena size is computed at compile time as:
     *   MaxElements * sizeof(T)
     */
    explicit MyMapAllocator()
    requires (MaxElements > 0)
    {
        state_ = std::make_shared<AllocatorState>(MaxElements);

        constexpr std::size_t arena_bytes = MaxElements * sizeof(T);
        arena_ = std::make_shared<Arena>(arena_bytes);
    }

    /**
     * @brief Constructor for expandable mode.
     *
     * Enabled only when MaxElements == 0.
     *
     * No element limit is enforced.
     * Initial arena size is specified in number of elements.
     */
    explicit MyMapAllocator(my_allocator::ExpandableArenaInitialCapacity elements)
    requires (MaxElements == 0)
    {
        state_ = std::make_shared<AllocatorState>();  // unlimited logical capacity

        std::size_t arena_bytes = elements.value * sizeof(T);
        arena_ = std::make_shared<Arena>(arena_bytes);
    }

    /**
     * @brief Copy constructor for rebind compatibility.
     *
     * Shares arena and state with other allocator.
     * Required for STL allocator-aware containers.
     */
    template<typename U>
    explicit MyMapAllocator(const MyMapAllocator<U, MaxElements>& other) noexcept
            : state_(other.state_),
              arena_(other.arena_)
    {}

    /**
     * @brief Allocates memory for n objects of type T.
     *
     * In fixed mode:
     *   Throws std::bad_alloc if element limit is exceeded.
     *
     * In expandable mode:
     *   No element limit is enforced.
     */
    T* allocate(std::size_t n)
    {
        if constexpr (MaxElements != 0)
        {
            if (state_->allocated_ + n > state_->max_elements_)
                throw std::bad_alloc{};
        }

        void* ptr = arena_->allocate_bytes(
                n * sizeof(T),
                alignof(T)
        );

        state_->allocated_ += n;
        return static_cast<T*>(ptr);
    }

    /**
     * @brief Deallocate memory for n objects.
     *
     * Physical memory is not returned to the arena.
     * Logical allocation counter is not decreased here
     * because the arena follows monotonic allocation model.
     */
    void deallocate(T*, std::size_t) noexcept {}

    template<typename U, std::size_t M>
    friend class MyMapAllocator;

    template<typename T1, std::size_t M1,
            typename T2, std::size_t M2>
    friend bool operator==(const MyMapAllocator<T1, M1>&,
                           const MyMapAllocator<T2, M2>&) noexcept;
};


/**
 * @brief Allocator equality comparison.
 *
 * Two allocators are equal if they share the same arena.
 */
template<typename T1, std::size_t M1,
        typename T2, std::size_t M2>
bool operator==(const MyMapAllocator<T1, M1>& a,
                const MyMapAllocator<T2, M2>& b) noexcept
{
    return a.arena_ == b.arena_;
}

template<typename T1, std::size_t M1,
        typename T2, std::size_t M2>
bool operator!=(const MyMapAllocator<T1, M1>& a,
                const MyMapAllocator<T2, M2>& b) noexcept
{
    return !(a == b);
}
