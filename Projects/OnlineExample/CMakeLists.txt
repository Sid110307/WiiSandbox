cmake_minimum_required(VERSION 3.20)
project(OnlineExample)
set(TARGET ${PROJECT_NAME})

file(GLOB_RECURSE SOURCES src/template.c)
file(GLOB_RECURSE BINFILES data/*.*)

add_executable(${TARGET}.elf ${SOURCES})

target_link_libraries(${TARGET}.elf wiiuse bte ogc m)
set_target_properties(${TARGET}.elf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
file(COPY ${PROJECT_SOURCE_DIR}/meta.xml ${BINFILES} DESTINATION ${PROJECT_SOURCE_DIR}/bin)
add_custom_target(${TARGET}_run COMMAND wiiload ${PROJECT_SOURCE_DIR}/bin/${TARGET}.dol DEPENDS ${TARGET})
