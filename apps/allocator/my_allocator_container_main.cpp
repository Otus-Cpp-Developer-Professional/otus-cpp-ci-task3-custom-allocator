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

    std::cout << "\n";


    // ============================================================
    // Fixed policy example
    // ============================================================

    using FixedMapPolicy =
            my_allocator::policy::Fixed<16>;

    using FixedMapAllocator =
            MyMapAllocator<
                    std::pair<const int, int>,
                    FixedMapPolicy
            >;

    std::map<int, int, std::less<int>, FixedMapAllocator>
            fixed_map;

    for (int i = 0; i < 10; ++i)
        fixed_map.emplace(i, factorial(i));

    std::cout << "std::map with fixed allocator (limit = 16):\n";
    for (const auto& [key, value] : fixed_map)
        std::cout << key << " " << value << '\n';

    std::cout << "\n";


    // ============================================================
    // Expandable policy example (std::map)
    // ============================================================

    using ExpandableMapPolicy =
            my_allocator::policy::Expandable<4>;
    // initial capacity = 4 elements
    // no logical limit

    using ExpandableMapAllocator =
            MyMapAllocator<
                    std::pair<const int, int>,
                    ExpandableMapPolicy
            >;

    std::map<int, int, std::less<int>, ExpandableMapAllocator>
            expandable_map;

    // Вставляем больше, чем initial (4)
    // Логического лимита нет — арена должна расширяться
    for (int i = 0; i < 10; ++i)
        expandable_map.emplace(i, factorial(i));

    std::cout << "std::map with expandable allocator (initial = 4):\n";
    for (const auto& [key, value] : expandable_map)
        std::cout << key << " " << value << '\n';

    std::cout << "\n";


    // ============================================================
    // MyContainer with expandable allocator
    // ============================================================

    using ExpandableContainerPolicy =
            my_allocator::policy::Expandable<2>;
    // намеренно маленький initial

    using ExpandableContainerAllocator =
            MyMapAllocator<
                    int,
                    ExpandableContainerPolicy
            >;

    MyContainer<int, ExpandableContainerAllocator>
            expandable_container;

    // Вставляем больше initial (2)
    // Контейнер должен продолжить работу без bad_alloc
    for (int i = 0; i < 10; ++i)
        expandable_container.push_back(i);

    std::cout << "MyContainer with expandable allocator (initial = 2):\n";
    for (int v : expandable_container)
        std::cout << v << '\n';

    std::cout << "\n";

    int b;
    std::cin >> b;
    return 0;
}
