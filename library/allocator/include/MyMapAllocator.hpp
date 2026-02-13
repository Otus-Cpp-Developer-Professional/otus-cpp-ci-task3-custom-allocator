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
         * Stores logical allocation accounting shared between all
         * allocator copies referring to the same arena.
         *
         * - max_elements_ – maximum number of elements allowed
         *                   (0 means unlimited)
         * - allocated_    – number of elements logically allocated
         *
         * Copies of the allocator share this state via std::shared_ptr,
         * meaning allocation limits are shared across containers
         * using the same allocator instance.
         */
        struct AllocatorState {

            std::size_t max_elements_ = 0;
            std::size_t allocated_    = 0;

            AllocatorState() = default;

            explicit AllocatorState(std::size_t max_elements)
                    : max_elements_(max_elements) {}
        };
    }


    namespace policy {

        /**
         * @brief Fixed-capacity policy.
         *
         * @tparam Max     Maximum number of elements allowed.
         * @tparam Initial Initial arena capacity in elements.
         *
         * Max and Initial are compile-time constants.
         * Typically Initial == Max.
         */
        template<std::size_t Max, std::size_t Initial = Max>
        struct Fixed {
            static constexpr std::size_t max     = Max;
            static constexpr std::size_t initial = Initial;
        };

        /**
         * @brief Expandable policy.
         *
         * @tparam Initial Initial arena capacity in elements.
         *
         * No logical element limit is enforced (max == 0).
         * Arena may grow internally when capacity is exceeded.
         */
        template<std::size_t Initial = 1024>
        struct Expandable {
            static constexpr std::size_t max     = 0;
            static constexpr std::size_t initial = Initial;
        };

    }

}


/**
 * @brief STL-compatible arena-based allocator.
 *
 * Configuration is fully defined by the Policy type.
 *
 * The allocator behavior depends on Policy::max:
 *
 * 1. Fixed mode (Policy::max > 0)
 *    - Maximum number of elements is known at compile time.
 *    - Allocation beyond this limit throws std::bad_alloc.
 *
 * 2. Expandable mode (Policy::max == 0)
 *    - No logical element limit is enforced.
 *    - Arena may grow when needed.
 *
 * Initial arena capacity is defined by Policy::initial.
 *
 * Copies of the allocator share:
 *  - underlying Arena
 *  - logical allocation state
 *
 * Memory is reclaimed only when the last allocator copy is destroyed.
 *
 * @tparam T      Value type
 * @tparam Policy Compile-time configuration type
 *
 * @note Not thread-safe.
 */
template<
        typename T,
        typename Policy = my_allocator::policy::Expandable<>
>
class MyMapAllocator
{
public:

    using value_type = T;

    /**
     * @brief Propagation traits.
     *
     * Allocator is stateful and shared.
     * Container copy/move/swap propagates allocator.
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

    using Arena          = my_allocator::detail::Arena;
    using AllocatorState = my_allocator::detail::AllocatorState;

    static constexpr std::size_t MaxElements = Policy::max;
    static constexpr std::size_t Initial     = Policy::initial;

    /// Shared allocation accounting
    std::shared_ptr<AllocatorState> state_;

    /// Shared underlying memory arena
    std::shared_ptr<Arena> arena_;

public:

    /**
     * @brief Default constructor.
     *
     * Behavior depends entirely on Policy:
     *
     * - If Policy::max > 0 → fixed logical limit
     * - If Policy::max == 0 → unlimited logical capacity
     *
     * Arena initial size is Policy::initial * sizeof(T).
     */
    MyMapAllocator()
    {
        if constexpr (MaxElements > 0) {
            state_ = std::make_shared<AllocatorState>(MaxElements);
        } else {
            state_ = std::make_shared<AllocatorState>();
        }

        constexpr std::size_t arena_bytes =
                Initial * sizeof(T);

        arena_ = std::make_shared<Arena>(arena_bytes);
    }

    /**
     * @brief Converting copy constructor.
     *
     * Required for allocator rebinding performed by
     * std::allocator_traits.
     *
     * Shares arena and state between different T instantiations
     * of the same Policy.
     */
    template<typename U>
    MyMapAllocator(const MyMapAllocator<U, Policy>& other) noexcept
            : state_(other.state_),
              arena_(other.arena_)
    {}

    /**
     * @brief Allocates memory for n objects of type T.
     *
     * In fixed mode:
     *   Throws std::bad_alloc if logical limit is exceeded.
     *
     * In expandable mode:
     *   No logical limit check is performed.
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
     * Arena follows monotonic allocation model.
     */
    void deallocate(T*, std::size_t) noexcept {}

    /**
     * @brief Allocator equality.
     *
     * Two allocators are equal if they share the same arena.
     */
    bool operator==(const MyMapAllocator& other) const noexcept
    {
        return arena_ == other.arena_;
    }

    bool operator!=(const MyMapAllocator& other) const noexcept
    {
        return !(*this == other);
    }

    template<typename, typename>
    friend class MyMapAllocator;
};
