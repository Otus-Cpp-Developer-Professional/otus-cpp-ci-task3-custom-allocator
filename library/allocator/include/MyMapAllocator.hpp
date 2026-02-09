//
// Created by Sg on 07.02.2026.
//

#pragma once

#include <map>
#include <cstddef>
#include <new>
#include <memory>
#include <vector>

#include "detail/Arena.hpp"
/**
 * @brief Simple arena-based allocator for STL containers
 *
 * MyMapAllocator is a stateful allocator that allocates memory from
 * a private memory arena. The arena is created inside the allocator
 * and owns all allocated memory for the allocator's lifetime.
 *
 * This allocator:
 * - supports allocation of contiguous memory blocks
 * - respects alignment requirements (alignof(T))
 * - does not support deallocation of individual objects
 *
 * Memory is released only when the allocator (and its arena) is destroyed.
 * This design is suitable for containers with monotonic allocation patterns
 * (e.g. std::map built once and destroyed as a whole).
 *
 * @tparam T Type of objects to allocate
 *
 * @note deallocate() is intentionally a no-op
 * @note This allocator is not thread-safe
 */
template<class T>
struct MyMapAllocator
{
    using value_type = T;

    /// Shared ownership of the underlying memory arena
    std::shared_ptr<my_allocator::detail::Arena> arena_;

    /// Default constructor is disabled: arena size must be specified
    MyMapAllocator() = delete;

    /**
     * @brief Constructs allocator with a new memory arena
     *
     * @param arena_bytes Size of the arena in bytes
     */
    explicit MyMapAllocator(size_t arena_bytes)
    {
        arena_ = std::make_shared<my_allocator::detail::Arena>(arena_bytes);
    }

    /**
     * @brief Copy-construct allocator from another allocator of different type
     *
     * Copies the underlying arena pointer.
     * Required for STL allocator compatibility.
     *
     * @tparam U Allocated type of the source allocator
     * @param other Source allocator
     */
    template<class U>
    explicit MyMapAllocator(const MyMapAllocator<U>& other) noexcept
            : arena_(other.arena_) {}

    /**
     * @brief Allocates memory for n objects of type T
     *
     * @param n Number of objects
     * @return Pointer to allocated memory
     *
     * @throws std::bad_alloc if the arena cannot satisfy the request
     */
    T* allocate(std::size_t n)
    {
        return static_cast<T*>(
                arena_->allocate_bytes(n * sizeof(T), alignof(T))
        );
    }

    /**
     * @brief Deallocation function (no-op)
     *
     * This allocator does not support individual deallocation.
     * Memory is reclaimed only when the arena is destroyed.
     *
     * @param Pointer previously returned by allocate (ignored)
     * @param Number of objects (ignored)
     */
    void deallocate(T*, std::size_t) noexcept {}
};
