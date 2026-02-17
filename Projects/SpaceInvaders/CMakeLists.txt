cmake_minimum_required(VERSION 3.20)
project(SpaceInvaders)

set(TARGET ${PROJECT_NAME})
file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)
file(GLOB_RECURSE BINFILES data/*.*)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} grrlib freetype brotlidec brotlicommon bz2 fat jpeg pngu png z asnd mad wiiuse bte ogc m)

set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME boot
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    SUFFIX .dol
)
file(COPY ${PROJECT_SOURCE_DIR}/meta.xml ${BINFILES} DESTINATION ${PROJECT_SOURCE_DIR}/bin)
add_custom_target(${TARGET}_run COMMAND wiiload ${PROJECT_SOURCE_DIR}/bin/boot.dol DEPENDS ${TARGET})
