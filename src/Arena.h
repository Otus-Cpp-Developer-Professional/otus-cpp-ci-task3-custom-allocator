#pragma once

#include <cstddef>
#include <memory>
#include <vector>

class Arena
{
    struct Block
    {
        std::byte* buffer;
        std::size_t used = 0;
        std::size_t capacity;

        explicit Block(std::size_t cap)
                : buffer(static_cast<std::byte*>(::operator new(cap)))
                , capacity(cap)
        {}

        ~Block()
        {
            ::operator delete(buffer);
        }
    };

public:
    explicit Arena(std::size_t block_size);
    ~Arena() = default;


    void* allocate_bytes(std::size_t size, std::size_t alignment);

private:
    void add_block(std::size_t cap);

    std::vector<std::unique_ptr<Block>> blocks;
    std::size_t block_size;
};
