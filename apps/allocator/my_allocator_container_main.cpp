#include <iostream>
#include <map>

#include <MyMapAllocator.hpp>
#include <MyContainer.hpp>

int factorial(int n)
{
    int result = 1;
    for (int i = 1; i <= n; ++i)
        result *= i;
    return result;
}

int main()
{

    std::map<int, int> default_map;

    for (int i = 0; i < 10; ++i)
        default_map.emplace(i, factorial(i));

    std::cout << "std::map with default allocator:\n";
    for (const auto& [key, value] : default_map)
        std::cout << key << " " << value << '\n';

    std::cout << '\n';

    using MapAllocator =
            MyMapAllocator<
                    std::pair<const int, int>,
                    16
            >;
    // We need extra capacity for internal map allocations
    MapAllocator map_alloc{}; //not 10

    std::map<int, int, std::less<int>, MapAllocator>
            custom_map(std::less<int>{}, map_alloc);

    for (int i = 0; i < 10; ++i)
        custom_map.emplace(i, factorial(i));

    std::cout << "std::map with custom allocator:\n";
    for (const auto& [key, value] : custom_map)
        std::cout << key << " " << value << '\n';

    std::cout << '\n';

    MyContainer<int> default_container;

    for (int i = 0; i < 10; ++i)
        default_container.push_back(i);

    std::cout << "MyContainer with default allocator:\n";
    for (int v : default_container)
        std::cout << v << '\n';

    std::cout << '\n';


    using ContainerAllocator =
            MyMapAllocator<
                    int,
                    10
            >;

    // std::vector performs no internal allocations, so fixed allocator capacity
    // directly limits the number of stored elements
    ContainerAllocator container_alloc;

    MyContainer<int, ContainerAllocator>
            custom_container(container_alloc);

    for (int i = 0; i < 10; ++i)
        custom_container.push_back(i);

    std::cout << "MyContainer with custom allocator:\n";
    for (int v : custom_container)
        std::cout << v << '\n';


    return 0;
}
