
#include <iostream>
#include <string>

#include <MyMapAllocator.hpp>
#include <MyContainer.hpp>

struct OverAligned {

    std::uint64_t i;

    [[nodiscard]] std::string getI() const { return std::to_string(i);}

    explicit OverAligned(std::uint64_t _i) : i(_i){};
    OverAligned() = default;
};

int main()
{
    using value = OverAligned;

    MyMapAllocator<std::pair<const int, value>> alloc(4096);

    MyMapAllocator<int> alloc2(4096);

    MyContainer<OverAligned, decltype(alloc2)> cc{alloc2};



    std::map<
            int,
            value,
            std::less<int>,
            MyMapAllocator<std::pair<const int, value>>
    > m(std::less<int>{}, alloc);

    try{
        for(int i = 0; i < 100; i++)
        {
            m[i] = value(i);

            if(i % 2 == 0)
                cc.push_front(value(i));
            else
                cc.push_back(value(i));

         //   std::cout << i << std::endl;
        }
    } catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    int i = 0;

    for(auto a:cc)
    {
        std::cout << i << " " << a.getI() << std::endl;
        i++;
    }

    return 0;
}
