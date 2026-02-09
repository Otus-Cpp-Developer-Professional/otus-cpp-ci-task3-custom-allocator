find_package(Boost REQUIRED COMPONENTS unit_test_framework)

add_executable(test_container
        test_container.cpp
        )

target_link_libraries(test_container PRIVATE
        container
        Boost::unit_test_framework
        )

add_test(
        NAME container
        COMMAND test_container
)
