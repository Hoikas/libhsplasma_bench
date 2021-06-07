cmake_minimum_required(VERSION 3.12)
project(HSPlasmaBench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(cxxopts REQUIRED)
find_package(HSPlasma REQUIRED)
find_package(string_theory REQUIRED)

add_executable(HSPlasmaBench main.cpp)
target_link_libraries(HSPlasmaBench cxxopts::cxxopts HSPlasma string_theory)

if(MSVC)
    target_compile_options(HSPlasmaBench PRIVATE "/wd4251")
endif()

# If doing a manual HSPlasma build with vcpkg, the HSPlasma.dll will not be
# copied over by vcpkg. So, we fake it here.
if(DEFINED VCPKG_TOOLCHAIN AND VCPKG_APPLOCAL_DEPS)
    add_custom_target(
        HSPlasmaAppLocal ALL
        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:HSPlasma>" "$<TARGET_FILE_DIR:HSPlasmaBench>"
    )
endif()
