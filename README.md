\mainpage Custom STL-Compatible Allocator and Container

## Custom STL-Compatible Allocator and Container

This project implements a custom STL-compatible allocator and a simple container
parameterized by an allocator.

The allocator configuration is defined entirely at compile time using a
policy-based design.

## Allocator

`MyMapAllocator<T, Policy>` is a stateful allocator based on a byte-oriented
memory arena.

The allocator behavior is determined by the `Policy` type:

- `policy::Fixed<Max, Initial>`
  - Enforces a compile-time logical element limit (`Max`)
  - Allocation beyond this limit throws `std::bad_alloc`
  - Initial arena capacity is `Initial` elements

- `policy::Expandable<Initial>`
  - No logical element limit is enforced
  - The arena may grow when capacity is exceeded
  - Initial arena capacity is `Initial` elements

### Features

- Fully STL-compatible allocator interface
- Policy-based compile-time configuration
- Allocation is performed in **element units**
  (`allocate(n)` allocates memory for `n` elements)
- Shared logical allocation state between allocator copies
- Monotonic allocation model (individual deallocation is not supported)
- Memory is released when the last allocator instance is destroyed

### Important Note

STL containers (e.g. `std::map`) may perform internal allocations
for node management and rebalancing.

Therefore, fixed-capacity mode limits the total number of allocation
operations, not strictly the number of user-visible elements.

## Custom Container

`MyContainer<T, Allocator>` is a simple singly linked container that:

- Is parameterized by an allocator (similar to STL containers)
- Supports element insertion (`push_back`, `push_front`)
- Supports forward iteration
- Implements `begin()`, `end()`, `size()`, and `empty()`

The container works with both `std::allocator` and `MyMapAllocator`.

## Demo Application

The demo application demonstrates:

- `std::map<int, int>` with default allocator
- `std::map<int, int>` with fixed policy allocator
- `std::map<int, int>` with expandable policy allocator
- `MyContainer<int>` with default allocator
- `MyContainer<int>` with fixed and expandable policies

Maps are filled with values `{0..9 → factorial}`.
Containers are filled with values `{0..9}`.

## Tests

Unit tests (Boost.Test) verify:

- Basic allocation behavior
- Correct alignment handling
- Fixed-capacity overflow (`std::bad_alloc`)
- Expandable arena growth
- Shared allocator state across copies
- Compatibility with `std::map`
- Correct interaction with STL allocator_traits rebinding

## Notes

This project is intended for educational purposes and demonstrates:

- Policy-based allocator design
- Stateful allocator semantics
- STL allocator requirements
- Integration with allocator-aware containers
