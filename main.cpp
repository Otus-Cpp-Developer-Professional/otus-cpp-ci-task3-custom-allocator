#include "lib.h"

#include <iostream>
#include "src/MyMapAllocator.hpp"

struct alignas(64) OverAligned {
    std::uint64_t x[10];
};

int main()
{
    using value = OverAligned;
    std::cout << "Version: " << version() << std::endl;

    MyMapAllocator<std::pair<const int, value>> alloc(1000000);

    std::map<
            int,
            value,
            std::less<int>,
            MyMapAllocator<std::pair<const int, value>>
    > m(std::less<int>{}, alloc);


    try{
        for(int i = 0; i < 100; i++)
        {
            m[i] = value();
            std::cout << "used " << i << " - " <<alloc.pool_->used << std::endl;
        }
    } catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }



    return 0;
}
