# Custom STL-Compatible Allocator and Container

This project implements a custom STL-compatible allocator and a simple container
parameterized by an allocator, as required by the assignment.

## Allocator

`MyMapAllocator` is a stateful allocator based on a byte-oriented memory arena.

Features:
- STL-compatible allocator interface
- Allocation is performed in **element units** (`allocate(n)` allocates `n` elements)
- **Fixed-capacity mode**: allocation beyond the specified element limit throws `std::bad_alloc`
- **Expandable mode** (optional): allocator may grow the underlying arena
- Individual deallocation is intentionally not supported
- Memory is released when the allocator is destroyed

Note:
STL containers (e.g. `std::map`) may perform internal allocations during construction,
so allocator capacity limits the number of allocation operations, not strictly the number
of user-visible elements.

## Custom Container

`MyContainer` is a simple singly linked container that:
- Is parameterized by an allocator (similar to STL containers)
- Supports element insertion (`push_back`, `push_front`)
- Supports forward iteration
- Implements `begin()`, `end()`, `size()`, and `empty()`

The container can be used with both `std::allocator` and the custom allocator.

## Demo Application

The demo application:
- Creates `std::map<int, int>` with default and custom allocators
- Fills maps with values `{0..9 → factorial}`
- Creates `MyContainer<int>` with default and custom allocators
- Inserts values `0..9`
- Prints all stored values to standard output

## Tests

Unit tests (Boost.Test) verify:
- Basic allocation and alignment
- Fixed-capacity behavior (`std::bad_alloc` on overflow)
- Optional expandable behavior
- Compatibility with `std::map`
- Allocator copy/rebind correctness

## Notes

This project is intended for educational purposes and demonstrates allocator mechanics,
allocator-aware containers, and STL integration.
