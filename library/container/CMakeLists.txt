# -------------------------------------------------
# Container (header-only)
# -------------------------------------------------

add_library(container INTERFACE)

# -------------------------------------------------
# Public include
# -------------------------------------------------

target_include_directories(container INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

# -------------------------------------------------
# Dependencies
# -------------------------------------------------

# Контейнер использует аллокатор
target_link_libraries(container INTERFACE allocator)

# -------------------------------------------------
# C++ standard
# -------------------------------------------------

target_compile_features(container INTERFACE cxx_std_20)
