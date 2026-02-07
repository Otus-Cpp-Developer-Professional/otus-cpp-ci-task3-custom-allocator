//
// Created by Sg on 07.02.2026.
//

#pragma once

#include <map>
#include <cstddef>
#include <new>
#include <memory>
#include <vector>

#include "Arena.h"

template<class T>
struct MyMapAllocator {
    using value_type = T;

    std::shared_ptr<Arena> arena_;

    MyMapAllocator() = default;

    explicit MyMapAllocator(std::shared_ptr<Arena> a)
            : arena_(std::move(a)) {}

    template<class U>
    explicit MyMapAllocator(const MyMapAllocator<U>& other) noexcept
            : arena_(other.arena_) {}

    T* allocate(std::size_t n) {
        return static_cast<T*>(
                arena_->allocate_bytes(n * sizeof(T), alignof(T))
        );
    }

    void deallocate(T*, std::size_t) noexcept {}
};







