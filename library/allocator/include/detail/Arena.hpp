#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>
#include <new>
#include <stdexcept>

namespace my_allocator::detail
{
    /**
     * @brief Monotonic memory arena for raw byte allocation
     *
     * Arena manages memory in a sequence of dynamically allocated blocks.
     * Memory is allocated linearly from each block and is never returned
     * back to the arena individually.
     *
     * When the current block cannot satisfy an allocation request,
     * a new block is allocated and appended to the arena.
     *
     * This class:
     * - supports aligned memory allocation
     * - grows dynamically by adding new blocks
     * - does not support deallocation of individual allocations
     *
     * All memory is released only when the Arena object is destroyed.
     *
     * @note This class is not thread-safe
     * @note Intended for use by custom allocators
     */
    class Arena
    {
        /**
         * @brief Internal memory block
         *
         * Represents a single contiguous memory buffer owned by the arena.
         * Memory inside the block is consumed linearly.
         */
        struct Block
        {
            /// Raw memory buffer
            std::byte* buffer;

            /// Number of bytes already used in the buffer
            std::size_t used = 0;

            /// Total buffer capacity in bytes
            std::size_t capacity;

            /**
             * @brief Constructs a block with given capacity
             *
             * @param cap Size of the block in bytes
             *
             * @throws std::bad_alloc if memory allocation fails
             */
            explicit Block(std::size_t cap)
                    : buffer(static_cast<std::byte*>(::operator new(cap)))
                    , capacity(cap)
            {}

            /**
             * @brief Releases the owned memory buffer
             */
            ~Block()
            {
                ::operator delete(buffer);
            }
        };

    public:
        /**
         * @brief Constructs arena with an initial block
         *
         * @param block_size Default size of newly allocated blocks (in bytes)
         */
        explicit Arena(std::size_t block_size)
                : block_size(block_size)
        {
            add_block(block_size);
        }

        /**
         * @brief Destroys the arena and releases all allocated memory
         */
        ~Arena() = default;

        /**
         * @brief Allocates a block of raw memory with specified alignment
         *
         * Attempts to allocate memory from the current block.
         * If there is not enough space or alignment cannot be satisfied,
         * a new block is allocated and the request is retried.
         *
         * @param size Number of bytes to allocate
         * @param alignment Required alignment (must be a power of two)
         *
         * @return Pointer to aligned memory block
         *
         * @throws std::invalid_argument if alignment is not a power of two
         * @throws std::bad_alloc if memory allocation fails
         */
        void* allocate_bytes(std::size_t size, std::size_t alignment)
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

                add_block((std::max)(block_size, size + alignment));
            }
        }

    private:
        /**
         * @brief Allocates and appends a new memory block
         *
         * @param cap Capacity of the new block in bytes
         */
        void add_block(std::size_t cap)
        {
            blocks.emplace_back(std::make_unique<Block>(cap));
        }

        /// List of allocated memory blocks
        std::vector<std::unique_ptr<Block>> blocks;

        /// Default block size for new blocks
        std::size_t block_size;
    };
}
