cmake_minimum_required(VERSION 3.20)
project(OnlineExample)

set(WIILOAD "tcp:192.168.0.113")
set(DEVKITPPC "/opt/devkitpro/devkitPPC")
set(TARGET ${PROJECT_NAME})

if (NOT DEFINED DEVKITPPC)
    message(FATAL_ERROR "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -Wall ${MACHDEP} ${INCLUDE}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g ${MACHDEP}")

include_directories(${DEVKITPPC}/include)
link_directories(${DEVKITPPC}/lib)

file(GLOB_RECURSE SOURCES src/template.c)
file(GLOB_RECURSE BINFILES data/*.*)

add_executable(${TARGET} ${SOURCES} ${BINFILES})

target_link_libraries(${TARGET} wiiuse bte ogc m)
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_custom_target(${TARGET}_copy_files COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/meta.xml ${CMAKE_SOURCE_DIR}/bin/meta.xml)
add_dependencies(${TARGET} ${TARGET}_copy_files)
add_custom_target(${TARGET}_run COMMAND wiiload ${CMAKE_SOURCE_DIR}/bin/${TARGET}.dol DEPENDS ${TARGET})
