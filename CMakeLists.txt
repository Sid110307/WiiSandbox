cmake_minimum_required(VERSION 3.20)
project(WiiSandbox)

set(WIILOAD "tcp:192.168.0.113")
set(DEVKITPPC "$ENV{DEVKITPRO}/devkitPPC")
set(DEPS "-DGEKKO -mrvl -mcpu=750 -meabi -mhard-float -Wl,-Map,${PROJECT_NAME}.elf.map")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -Wall ${DEPS}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g ${DEPS}")

include_directories(
        ${DEVKITPPC}/include
        $ENV{DEVKITPRO}/libogc/include
        $ENV{DEVKITPRO}/portlibs/ppc/include
        $ENV{DEVKITPRO}/portlibs/wii/include
)
link_directories(
        ${DEVKITPPC}/lib
        $ENV{DEVKITPRO}/libogc/lib/wii
        $ENV{DEVKITPRO}/portlibs/ppc/lib
        $ENV{DEVKITPRO}/portlibs/wii/lib
)

add_subdirectory(Projects/First)
add_subdirectory(Projects/OnlineExample)
add_subdirectory(Projects/Pong)
add_subdirectory(Projects/SpaceInvaders)
