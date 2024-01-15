#!/usr/bin/env python3

import os
import shutil
import sys

if os.geteuid() != 0:
    print("This script must be run as root.")
    exit(1)

DATA_EXTENSIONS = [".dol", ".xml", ".mp3", ".png", ".jpg", ".ttf"]
ROOT = "/media/usb"
LOCAL = os.path.join(os.path.expanduser("~sid"), "snap/dolphin-emulator/common/.local/share/dolphin-emu/Load/WiiSDSync")


def check_usb():
    if not os.path.exists(ROOT):
        print(f"USB drive not mounted ({ROOT}).")
        exit(1)


def help():
    print(f"Usage: {sys.argv[0]} <command>")
    print("Commands:")
    print("  list - List all games")
    print("  upload - Upload a game")
    print("  init - Create a new game")
    print("  help - Show this help message")


def main():
    if len(sys.argv) < 2:
        help()
        exit(1)

    if sys.argv[1] == "list":
        check_usb()

        for root_dir in {ROOT, LOCAL}:
            print(f"Games in {root_dir}:")

            try:
                game_dirs = [dir for dir in os.listdir(os.path.join(root_dir, "apps")) if
                             os.path.isdir(os.path.join(root_dir, "apps", dir))]
                game_files = []

                for game_dir in game_dirs:
                    bin_path = os.path.join(root_dir, "apps", game_dir)
                    elf_files = [file for file in os.listdir(bin_path) if file.endswith(".elf")]
                    game_files.extend(os.path.join(bin_path, elf) for elf in elf_files)

                for index, file in enumerate(game_files, start=1):
                    print(f"  {index}) {file.replace(root_dir, '').replace('/apps/', '')[:-9]}")
            except FileNotFoundError:
                print("  No games found.")

            print()
    elif sys.argv[1] == "upload":
        check_usb()

        game_dirs = [dir for dir in os.listdir("Games") if os.path.isdir(os.path.join("Games", dir))]
        game_files = []

        for game_dir in game_dirs:
            bin_path = os.path.join("Games", game_dir, "bin")
            elf_files = [file for file in os.listdir(bin_path if os.path.isdir(bin_path) else "Games") if
                         file.endswith(".elf")]
            game_files.extend(os.path.join(bin_path, elf) for elf in elf_files)

        for index, file in enumerate(game_files, start=1):
            print(f"{index}) {file.replace('Games/', '').replace('/bin', '')}")

        choice = int(input("Choose a game to upload: ")) - 1

        if 0 <= choice < len(game_files):
            selected_file = game_files[choice]

            for root_dir in {ROOT, LOCAL}:
                app_folder = os.path.join(root_dir, "apps",
                                          os.path.dirname(selected_file).replace("Games/", "").replace("/bin", ""))
                os.makedirs(app_folder, exist_ok=True)
                shutil.copy(selected_file, os.path.join(app_folder, os.path.basename(selected_file)))

                for file in os.listdir(os.path.dirname(selected_file)):
                    if any(file.endswith(ext) for ext in DATA_EXTENSIONS):
                        shutil.copy(os.path.join(os.path.dirname(selected_file), file), app_folder)
        else:
            print("Invalid choice.")
            exit(1)
    elif sys.argv[1] == "init":
        name = input("Game name: ")
        author = input("Author: ")

        os.makedirs(os.path.join("Games", name, "src"), exist_ok=True)
        open(os.path.join("Games", name, "src", "main.c"), "w").close()

        with open(os.path.join("Games", name, "CMakeLists.txt"), "w") as f:
            f.writelines("""cmake_minimum_required(VERSION 3.20)
project(""" + name + """)

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

file(GLOB_RECURSE SOURCES src/main.c)
file(GLOB_RECURSE BINFILES data/*.*)

add_executable(${TARGET} ${SOURCES} ${BINFILES})

target_link_libraries(${TARGET} wiiuse bte ogc m)
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_custom_target(${TARGET}_copy_files COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/meta.xml ${CMAKE_SOURCE_DIR}/bin/meta.xml)
add_dependencies(${TARGET} ${TARGET}_copy_files)
add_custom_target(${TARGET}_run COMMAND wiiload ${CMAKE_SOURCE_DIR}/bin/${TARGET}.dol DEPENDS ${TARGET})
""")

        with open(os.path.join("Games", name, "meta.xml"), "w") as f:
            f.writelines("""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
    <name>""" + name + """</name>
    <coder>""" + author + """</coder>
    <version>1.0</version>
    <short_description>""" + name + """</short_description>
    <long_description>""" + name + """</long_description>
</app>
""")
    elif sys.argv[1] == "help":
        help()
    else:
        print("Invalid command.")
        help()

        exit(1)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting...")
        exit(1)
