cmake_minimum_required(VERSION 3.20)
project(WiiSandbox)

set(WIILOAD "tcp:192.168.0.118" CACHE STRING "The IP address of the Wii")
if (NOT DEFINED ENV{DEVKITPRO})
    message(FATAL_ERROR "DEVKITPRO environment variable is not set")
endif ()

set(DEVKITPPC "$ENV{DEVKITPRO}/devkitPPC" CACHE STRING "The path to devkitPPC")
set(GEKKO_FLAGS -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float)

add_compile_options(-g -Os -Wall -Wextra ${GEKKO_FLAGS})
add_link_options(${GEKKO_FLAGS})

include_directories(
    $ENV{DEVKITPRO}/libogc/include
    $ENV{DEVKITPRO}/portlibs/ppc/include
    $ENV{DEVKITPRO}/portlibs/wii/include
)
link_directories(
    $ENV{DEVKITPRO}/libogc/lib/wii
    $ENV{DEVKITPRO}/portlibs/ppc/lib
    $ENV{DEVKITPRO}/portlibs/wii/lib
)

file(GLOB CHILDREN RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/Projects" "${CMAKE_CURRENT_SOURCE_DIR}/Projects/*")
foreach (child ${CHILDREN})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Projects/${child}/CMakeLists.txt")
        add_subdirectory("Projects/${child}")
    endif ()
endforeach ()

add_custom_target(build_all DEPENDS ${CHILDREN} COMMENT "Building all projects")
