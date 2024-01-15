cmake_minimum_required(VERSION 3.20)
project(OnlineExample)
set(TARGET ${PROJECT_NAME})

file(GLOB_RECURSE SOURCES src/template.c)
file(GLOB_RECURSE BINFILES data/*.*)

add_executable(${TARGET} ${SOURCES} ${BINFILES})

target_link_libraries(${TARGET} wiiuse bte ogc m)
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
file(COPY ${PROJECT_SOURCE_DIR}/meta.xml DESTINATION ${PROJECT_SOURCE_DIR}/bin)
add_custom_target(${TARGET}_run COMMAND wiiload ${PROJECT_SOURCE_DIR}/bin/${TARGET}.dol DEPENDS ${TARGET})
