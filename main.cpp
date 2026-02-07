#include "lib.h"

#include <iostream>
#include <memory>
#include "src/MyMapAllocator.hpp"

struct alignas(64) OverAligned {
    std::uint64_t x[10];
};

int main()
{
    using value = OverAligned;
    std::cout << "Version: " << version() << std::endl;

    auto a = std::make_shared<Arena>(4096);

    MyMapAllocator<std::pair<const int, value>> alloc(a);

    std::map<
            int,
            value,
            std::less<int>,
            MyMapAllocator<std::pair<const int, value>>
    > m(std::less<int>{}, alloc);

    try{
        for(int i = 0; i < 10000; i++)
        {
            m[i] = value();
            std::cout << i << std::endl;
        }
    } catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
