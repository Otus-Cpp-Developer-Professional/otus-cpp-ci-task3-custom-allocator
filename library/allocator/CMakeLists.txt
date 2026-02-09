# -------------------------------------------------
# Allocator (header-only)
# -------------------------------------------------

add_library(allocator INTERFACE)

# -------------------------------------------------
# Include directories
# -------------------------------------------------

target_include_directories(allocator
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

# -------------------------------------------------
# C++ standard
# -------------------------------------------------

target_compile_features(allocator INTERFACE cxx_std_20)

# -------------------------------------------------
# Warnings (аккуратно, per-target)
# -------------------------------------------------

if (MSVC)
    target_compile_options(allocator INTERFACE /W4)
else()
    target_compile_options(allocator INTERFACE -Wall -Wextra -pedantic)
endif()

# -------------------------------------------------
# Library properties (опционально)
# -------------------------------------------------

set_target_properties(allocator PROPERTIES
        OUTPUT_NAME "allocator"
        )
