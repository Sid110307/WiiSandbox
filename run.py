#!/usr/bin/env python3

import os
import shutil
import sys

if os.geteuid() != 0:
    print("This script must be run as root.")
    exit(1)

root = "/media/usb"
local = os.path.join(os.path.expanduser("~"), "snap/dolphin-emulator/common/.local/share/dolphin-emu/Load/WiiSDSync")


def check_usb():
    if not os.path.exists(root):
        print(f"USB drive not mounted ({root}).")
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

        for root_dir in {root, local}:
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
                    print(f"  {index}) {file.replace(root_dir, '').replace('/bin', '')}")
            except FileNotFoundError:
                print("  No games found.")

            print()
    elif sys.argv[1] == "upload":
        check_usb()

        game_dirs = [dir for dir in os.listdir("Games") if os.path.isdir(os.path.join("Games", dir))]
        game_files = []

        for game_dir in game_dirs:
            bin_path = os.path.join("Games", game_dir, "bin")
            elf_files = [file for file in os.listdir(bin_path) if file.endswith(".elf")]
            game_files.extend(os.path.join(bin_path, elf) for elf in elf_files)

        for index, file in enumerate(game_files, start=1):
            print(f"{index}) {file.replace('Games/', '').replace('/bin', '')}")

        choice = int(input("Choose a game to upload: ")) - 1

        if 0 <= choice < len(game_files):
            selected_file = game_files[choice]

            for root_dir in {root, local}:
                app_folder = os.path.join(root_dir, "apps",
                                          os.path.dirname(selected_file).replace("Games/", "").replace("/bin", ""))
                os.makedirs(app_folder, exist_ok=True)
                shutil.copy(selected_file, os.path.join(app_folder, os.path.basename(selected_file)))

                for file in os.listdir(os.path.dirname(selected_file)):
                    if file.endswith(".dol") or file.endswith(".xml"):
                        shutil.copy(os.path.join(os.path.dirname(selected_file), file), app_folder)
        else:
            print("Invalid choice.")
            exit(1)
    elif sys.argv[1] == "init":
        name = input("Game name: ")
        author = input("Author: ")

        os.makedirs(os.path.join("Games", name, "src"), exist_ok=True)
        open(os.path.join("Games", name, "src", "main.c"), "w").close()

        with open(os.path.join("Games", name, "Makefile"), "w") as f:
            f.writelines("""export WIILOAD = tcp:192.168.0.113
export DEVKITPPC = /opt/devkitpro/devkitPPC

.SUFFIXES:

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

TARGET := $(notdir $(CURDIR))
BUILD := bin
SOURCES := src
DATA := data
INCLUDES :=

CFLAGS = -g -O2 -Wall $(MACHDEP) $(INCLUDE)
CXXFLAGS = $(CFLAGS)
LDFLAGS = -g $(MACHDEP) -Wl,-Map,$(notdir $@).map

LIBS := -lwiiuse -lbte -logc -lm
LIBDIRS :=

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
	export LD := $(CC)
else
	export LD := $(CXX)
endif

export OFILES_BIN := $(addsuffix .o,$(BINFILES))
export OFILES_SOURCES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)
export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)
export HFILES := $(addsuffix .h,$(subst .,_,$(BINFILES)))
export INCLUDE := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include) \\
				-I$(CURDIR)/$(BUILD) -I$(LIBOGC_INC)
export LIBPATHS := -L$(LIBOGC_LIB) $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export OUTPUT := $(CURDIR)/$(TARGET)

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

run:
	wiiload $(TARGET).dol

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
$(OFILES_SOURCES): $(HFILES)

%.jpg.o %_jpg.h: %.jpg
	$(bin2o)

-include $(DEPENDS)

endif

.PHONY: all $(BUILD) clean
all: $(BUILD)
	sudo mv $(OUTPUT).elf $(BUILD)/boot.elf
	sudo mv $(OUTPUT).dol $(BUILD)/$(TARGET).dol
	sudo cp meta.xml $(BUILD)/meta.xml
	@echo "===== Done! Now run \\"make run\\" to upload the program to the Wii. ====="
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
