cmake_minimum_required(VERSION 3.12)

# =========================
# PROJECT NAME
# =========================
set(PROJECT_NAME_VAR custom_allocator)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(${PROJECT_NAME_VAR} VERSION ${PROJECT_VERSION})

option(WITH_BOOST_TEST "Whether to build Boost test" ON)

configure_file(version.h.in version.h)

# -------------------------
# Targets
# -------------------------
add_executable(${PROJECT_NAME_VAR}_cli main.cpp)
add_library(${PROJECT_NAME_VAR} lib.cpp src/MyMapAllocator.hpp src/Arena.h src/Arena.cpp)

set_target_properties(${PROJECT_NAME_VAR}_cli ${PROJECT_NAME_VAR} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME_VAR}
    PRIVATE "${CMAKE_BINARY_DIR}"
)

target_link_libraries(${PROJECT_NAME_VAR}_cli PRIVATE
    ${PROJECT_NAME_VAR}
)

# -------------------------
# Tests
# -------------------------
if(WITH_BOOST_TEST)
    find_package(Boost COMPONENTS unit_test_framework REQUIRED)

    add_executable(test_version test_version.cpp)

    set_target_properties(test_version PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )

    target_compile_definitions(test_version PRIVATE BOOST_TEST_DYN_LINK)
    target_include_directories(test_version PRIVATE ${Boost_INCLUDE_DIR})

    target_link_libraries(test_version
        ${Boost_LIBRARIES}
        ${PROJECT_NAME_VAR}
    )
endif()

# -------------------------
# Warnings
# -------------------------
if (MSVC)
    target_compile_options(${PROJECT_NAME_VAR}_cli PRIVATE /W4)
    target_compile_options(${PROJECT_NAME_VAR} PRIVATE /W4)
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE /W4)
    endif()
else ()
    target_compile_options(${PROJECT_NAME_VAR}_cli PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(${PROJECT_NAME_VAR} PRIVATE -Wall -Wextra -pedantic -Werror)
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE -Wall -Wextra -pedantic -Werror)
    endif()
endif()

# -------------------------
# Install
# -------------------------
install(TARGETS ${PROJECT_NAME_VAR}_cli RUNTIME DESTINATION bin)

# -------------------------
# CPack
# -------------------------
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)

# -------------------------
# CTest
# -------------------------
if(WITH_BOOST_TEST)
    enable_testing()
    add_test(NAME test_version COMMAND test_version)
endif()
