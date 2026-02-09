find_package(Boost REQUIRED COMPONENTS unit_test_framework)

add_executable(test_allocator
        test_allocator.cpp
        )

target_link_libraries(test_allocator PRIVATE
        allocator
        Boost::unit_test_framework
        )

add_test(
        NAME allocator
        COMMAND test_allocator
)
