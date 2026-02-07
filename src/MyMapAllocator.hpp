//
// Created by Sg on 07.02.2026.
//

#pragma once
#include <map>
#include <cstddef>
#include <new>
#include <memory>

struct Pool {

    std::byte* buffer;
    size_t used;
    size_t capacity;

    ~Pool() {
        ::operator delete(buffer);
    }

};

template<typename T>
class MyMapAllocator {

public:

    using value_type = T;

    std::shared_ptr<Pool> pool_;

    template<class U>
    explicit MyMapAllocator(const MyMapAllocator<U>& other) noexcept
            : pool_(other.pool_) {}

    explicit MyMapAllocator(size_t capacity);
    ~MyMapAllocator();
    T* allocate(size_t n);

    void deallocate([[maybe_unused]] T* p,
                    [[maybe_unused]] std::size_t n) noexcept
    {}

};


template<typename T>
MyMapAllocator<T>::~MyMapAllocator() = default;

template<typename T>
MyMapAllocator<T>::MyMapAllocator(size_t capacity)   {

    pool_ = std::make_shared<Pool>();
    pool_->capacity = capacity;
    pool_->used = 0;
    pool_->buffer = static_cast<std::byte *>(::operator new(capacity));
}

template<typename T>
T* MyMapAllocator<T>::allocate(size_t n)
{
    std::size_t space = pool_->capacity - pool_->used;
    void* ptr = pool_->buffer + pool_->used;

    if (!std::align(alignof(T), n * sizeof(T), ptr, space))
        throw std::bad_alloc{};

    pool_->used = pool_->capacity - space + n * sizeof(T);
    return static_cast<T*>(ptr);
}






