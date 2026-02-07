#include "Arena.h"

#include <algorithm>
#include <new>
#include <stdexcept>



Arena::Arena(std::size_t block_size)
        : block_size(block_size)
{
    add_block(block_size);
}

void* Arena::allocate_bytes(std::size_t size, std::size_t alignment)
{

    if (alignment == 0 || (alignment & (alignment - 1)) != 0)
        throw std::invalid_argument("alignment must be power of two");

    for (;;)
    {
        Block* b = blocks.back().get();

        void* ptr = b->buffer + b->used;
        std::size_t space = b->capacity - b->used;

        if (std::align(alignment, size, ptr, space))
        {
            b->used = b->capacity - space + size;
            return ptr;
        }

        add_block(std::max(block_size, size + alignment));
    }
}


void Arena::add_block(std::size_t cap)
{
    blocks.emplace_back(std::make_unique<Block>(cap));
}
