cmake_minimum_required(VERSION 3.14)

# -------------------------------------------------
# Project / Version
# -------------------------------------------------

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")

project(otus_cpp_allocator
        VERSION 0.0.${PATCH_VERSION}
        LANGUAGES CXX
        )

configure_file(
        ${CMAKE_SOURCE_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/version.h
)

# -------------------------------------------------
# Options
# -------------------------------------------------

option(WITH_BOOST_TEST "Build tests" ON)

# -------------------------------------------------
# CPack
# -------------------------------------------------

set(CPACK_PACKAGE_NAME "otus-cpp-allocator")
set(CPACK_PACKAGE_VENDOR "OTUS")
set(CPACK_PACKAGE_CONTACT "sagge14@yandex.ru")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OTUS")

# Версия пакета берётся из project()
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

# -------------------------------------------------
# Global include (только generated files)
# -------------------------------------------------

# version.h доступен всем таргетам
include_directories(${CMAKE_BINARY_DIR})

# -------------------------------------------------
# Subprojects
# -------------------------------------------------

add_subdirectory(library/allocator)
add_subdirectory(library/container)
add_subdirectory(apps/allocator)

if(WITH_BOOST_TEST)
    enable_testing()
    add_subdirectory(library/allocator/test)
    add_subdirectory(library/container/test)
endif()

# -------------------------------------------------
# CPack
# -------------------------------------------------

include(CPack)
