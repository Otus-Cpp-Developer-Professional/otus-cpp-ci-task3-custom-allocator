# -------------------------------------------------
# Allocator demo application
# -------------------------------------------------

add_executable(allocator_app
        my_allocator_container_main.cpp
        )

# -------------------------------------------------
# Link libraries
# -------------------------------------------------

target_link_libraries(allocator_app PRIVATE
        allocator
        container
        )

# -------------------------------------------------
# C++ standard
# -------------------------------------------------

target_compile_features(allocator_app PRIVATE cxx_std_20)
