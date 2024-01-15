#!/usr/bin/env python3

import os
import shutil
import sys

if os.geteuid() != 0:
    print("This script must be run as root.")
    exit(1)

DATA_EXTENSIONS = [".dol", ".xml", ".mp3", ".png", ".jpg", ".ttf"]
ROOT = "/media/usb"
PROJECT_DIR = "Projects"
LOCAL = os.path.join(os.path.expanduser("~sid"), "snap/dolphin-emulator/common/.local/share/dolphin-emu/Load/WiiSDSync")


def check_usb():
    if not os.path.exists(ROOT):
        print(f"USB drive not mounted ({ROOT}).")
        exit(1)


def usage():
    print(f"Usage: {sys.argv[0]} <command>")
    print("Commands:")
    print("  list - List all projects")
    print("  upload - Upload a project")
    print("  init - Create a new project")
    print("  help - Show this help message")


def upload_project(selection):
    for root_dir in [ROOT, LOCAL]:
        destination = os.path.join(root_dir, "apps", os.path.dirname(selection)
                                   .replace(f"{PROJECT_DIR}/", "").replace("/bin", ""))
        os.makedirs(destination, exist_ok=True)
        shutil.copy(selection, os.path.join(destination, os.path.basename(selection)))

        for file in os.listdir(os.path.dirname(selection)):
            if any(file.endswith(ext) for ext in DATA_EXTENSIONS):
                shutil.copy(os.path.join(os.path.dirname(selection), file), destination)

        print(f"Uploaded {os.path.basename(selection)} to {destination}.")


def main():
    if len(sys.argv) < 2:
        usage()
        exit(1)

    if sys.argv[1] == "list":
        check_usb()

        for root_dir in [ROOT, LOCAL]:
            print(f"Projects in {root_dir}:")

            try:
                project_dirs = [directory for directory in os.listdir(os.path.join(root_dir, "apps")) if
                                os.path.isdir(os.path.join(root_dir, "apps", directory))]
                project_files = []

                for project_dir in project_dirs:
                    bin_path = os.path.join(root_dir, "apps", project_dir)
                    elf_files = [file for file in os.listdir(bin_path) if file.endswith(".elf")]
                    project_files.extend(os.path.join(bin_path, elf) for elf in elf_files)

                for index, file in enumerate(project_files, start=1):
                    f = file.replace(root_dir, '').replace('/apps/', '').replace(os.path.basename(file), '')[:-1]
                    print(f"  {index}) {f}")
            except FileNotFoundError:
                print("  No projects found.")

            print()
    elif sys.argv[1] == "upload":
        check_usb()

        project_dirs = [directory for directory in os.listdir(PROJECT_DIR) if
                        os.path.isdir(os.path.join(PROJECT_DIR, directory))]
        project_files = []

        for project_dir in project_dirs:
            bin_path = os.path.join(PROJECT_DIR, project_dir, "bin")
            elf_files = [file for file in os.listdir(bin_path if os.path.isdir(bin_path) else PROJECT_DIR) if
                         file.endswith(".elf")]
            project_files.extend(os.path.join(bin_path, elf) for elf in elf_files)

        for index, file in enumerate(project_files, start=1):
            f = file.replace(PROJECT_DIR, '').replace('/bin/', '').replace(os.path.basename(file), '')[1:]
            print(f"  {index}) {f}")

        choice = ""
        while not choice.isdigit() or int(choice) < 1 or int(choice) > len(project_files):
            choice = input("Choose a project to upload: ")

            if not choice.isdigit() or int(choice) < 1 or int(choice) > len(project_files):
                print("Invalid choice.")
            else:
                selection = project_files[int(choice) - 1]
                upload_project(selection)
    elif sys.argv[1] == "upload-all":
        check_usb()

        project_dirs = [directory for directory in os.listdir(PROJECT_DIR) if
                        os.path.isdir(os.path.join(PROJECT_DIR, directory))]
        project_files = []

        for project_dir in project_dirs:
            bin_path = os.path.join(PROJECT_DIR, project_dir, "bin")
            elf_files = [file for file in os.listdir(bin_path if os.path.isdir(bin_path) else PROJECT_DIR) if
                         file.endswith(".elf")]
            project_files.extend(os.path.join(bin_path, elf) for elf in elf_files)

        for file in project_files:
            upload_project(file)
    elif sys.argv[1] == "init":
        name = input("Project name: ")
        author = input("Author: ")

        os.makedirs(os.path.join(PROJECT_DIR, name, "src"), exist_ok=True)
        open(os.path.join(PROJECT_DIR, name, "src", "main.c"), "w").close()

        with open("CMakeLists.txt", "a") as f:
            f.write(f"add_subdirectory(Projects/{name})\n")

        with open(os.path.join(PROJECT_DIR, name, "CMakeLists.txt"), "w") as f:
            f.write("cmake_minimum_required(VERSION 3.20)\n")
            f.write(f"project({name})\n")
            f.write("set(TARGET ${PROJECT_NAME})\n\n")
            f.write("file(GLOB_RECURSE SOURCES src/main.c)\n")
            f.write("file(GLOB_RECURSE BINFILES data/*.*)\n\n")
            f.write("add_executable(${TARGET}.elf ${SOURCES})\n\n")
            f.write("target_link_libraries(${TARGET}.elf wiiuse bte ogc m)\n")
            f.write("set_target_properties(${TARGET}.elf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)"
                    "\n")
            f.write("file(COPY ${PROJECT_SOURCE_DIR}/meta.xml ${BINFILES} DESTINATION ${PROJECT_SOURCE_DIR}/bin)\n")
            f.write("add_custom_target(${TARGET}_run COMMAND wiiload ${PROJECT_SOURCE_DIR}/bin/${TARGET}.dol DEPENDS "
                    "${TARGET})\n")

        with open(os.path.join(PROJECT_DIR, name, "meta.xml"), "w") as f:
            f.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n")
            f.write("<app version=\"1\">\n")
            f.write(f"\t<name>{name}</name>\n")
            f.write(f"\t<coder>{author}</coder>\n")
            f.write("\t<version>1.0</version>\n")
            f.write("\t<short_description>Short description</short_description>\n")
            f.write("\t<long_description>Long description</long_description>\n")
            f.write("</app>\n")
    elif sys.argv[1] == "help":
        usage()
    else:
        print("Invalid command.")
        usage()

        exit(1)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting...")
        exit(1)
