#!/usr/bin/env python3

import argparse
import os
import shutil
import sys
from datetime import date
from pathlib import Path

try:
    from PIL import Image, ImageDraw
except ImportError:
    Image = None
    ImageDraw = None

    print("PIL (Pillow) not found. Icon generation will be disabled. Install with 'pip install Pillow'.")

DATA_EXTENSIONS = {".dol", ".xml", ".mp3", ".png", ".jpg", ".jpeg", ".ttf"}
PROJECT_DIR = Path("Projects")


def ensure_remote(root):
    if not root.exists():
        print(f"Wii SD/USB not mounted ({root}).")
        sys.exit(1)
    (root / "apps").mkdir(parents=True, exist_ok=True)


def get_windows_remote():
    for letter in "DEFGHIJKLMNOPQRSTUVWXYZ":
        p = Path(f"{letter}:\\")
        if p.exists():
            return p
    return Path("D:\\")


def local_dols():
    dols = []
    if not PROJECT_DIR.exists():
        return []

    for project in sorted([p for p in PROJECT_DIR.iterdir() if p.is_dir()], key=lambda p: p.name.lower()):
        bin_dir = project / "bin"
        if not bin_dir.is_dir():
            continue

        for dol in sorted(bin_dir.glob("*.dol"), key=lambda p: p.name.lower()):
            dols.append(dol)
    return dols


def remote_projects(root):
    apps = root / "apps"
    if not apps.exists():
        return []

    out = []
    for d in sorted([p for p in apps.iterdir() if p.is_dir()], key=lambda p: p.name.lower()):
        boot = d / "boot.dol"
        if boot.is_file():
            out.append((d.name, [boot]))
    return out


def upload_one(dol_path, root):
    if os.name not in {"nt", "dos"} and os.geteuid() != 0:
        print("This command must be run as root.")
        sys.exit(1)

    bin_dir = dol_path.parent.parent / "bin"
    data_dir = dol_path.parent.parent / "data"

    dest = root / "apps" / dol_path.parent.parent.name
    dest.mkdir(parents=True, exist_ok=True)
    shutil.copy2(dol_path, dest / "boot.dol")

    copied = set()
    for p in sorted((p for p in bin_dir.iterdir() if p.is_file()), key=lambda project: project.name.lower()):
        if p == dol_path:
            continue
        if p.suffix.lower() in DATA_EXTENSIONS:
            shutil.copy2(p, dest / p.name)
            copied.add(p.name.lower())

    if data_dir.is_dir():
        for p in sorted((p for p in data_dir.iterdir() if p.is_file()), key=lambda project: project.name.lower()):
            if p.suffix.lower() in DATA_EXTENSIONS and p.name.lower() not in copied:
                shutil.copy2(p, dest / p.name)

    print(f"Uploaded project to {dest}.")


def generate_icon(path, name):
    if Image is None or ImageDraw is None:
        return

    width, height = 128, 48
    img = Image.new("RGB", (width, height))
    draw = ImageDraw.Draw(img)

    top = (35, 95, 200)
    bottom = (20, 45, 110)
    for y in range(height):
        t = y / (height - 1)
        r = int(top[0] + (bottom[0] - top[0]) * t)
        g = int(top[1] + (bottom[1] - top[1]) * t)
        b = int(top[2] + (bottom[2] - top[2]) * t)
        draw.line([(0, y), (width, y)], fill=(r, g, b))
    draw.rectangle([0, 0, width - 1, height - 1], outline=(10, 20, 40))

    text = name[:20]
    bbox = draw.textbbox((0, 0), text)
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    x = (width - tw) // 2
    y = (height - th) // 2

    draw.text((x + 1, y + 1), text, fill=(0, 0, 0))
    draw.text((x, y), text, fill=(255, 255, 255))
    img.save(path)


def cmd_list(root):
    ensure_remote(root)
    projects = remote_projects(root)
    print(f"Projects in {root}:")

    if not projects:
        print("  No projects found.")
        print()

        return

    i = 1
    for name, dols in projects:
        for _ in dols:
            print(f"  {i}) {name}")
            i += 1
    print()


def cmd_upload(root):
    ensure_remote(root)
    dols = local_dols()

    if not dols:
        print(f"No local projects found in {PROJECT_DIR}/<name>/bin/*.dol")
        return
    for i, dol in enumerate(dols, start=1):
        print(f"  {i}) {dol.parent.parent.name} ({dol.name})")
    print()

    choice = ""
    while True:
        choice = input("Choose a project to upload: ").strip()
        if choice.isdigit():
            idx = int(choice)
            if 1 <= idx <= len(dols):
                upload_one(dols[idx - 1], root)
                break
        print("Invalid choice.")


def cmd_upload_all(root):
    ensure_remote(root)
    dols = local_dols()

    if not dols:
        print(f"No local projects found in {PROJECT_DIR}/<name>/bin/*.dol")
        return
    for dol in dols:
        upload_one(dol, root)


def write_file(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def cmd_init():
    name = input("Project name: ")
    name = "".join(c if c.isalnum() else "_" for c in name).strip("_") or "NewProject"
    author = input("Author: ").strip()

    proj = PROJECT_DIR / name
    (proj / "src").mkdir(parents=True, exist_ok=True)
    (proj / "bin").mkdir(parents=True, exist_ok=True)
    (proj / "data").mkdir(parents=True, exist_ok=True)

    source = proj / "src" / "main.c"
    if not source.exists():
        source.touch()

    proj_cmake = f"""cmake_minimum_required(VERSION 3.20)
project({name})

set(TARGET ${{PROJECT_NAME}})
file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)
file(GLOB_RECURSE BINFILES data/*.*)

add_executable(${{TARGET}} ${{SOURCES}})
target_link_libraries(${{TARGET}} wiiuse bte ogc m)

set_target_properties(${{TARGET}} PROPERTIES
    OUTPUT_NAME boot
    RUNTIME_OUTPUT_DIRECTORY ${{PROJECT_SOURCE_DIR}}/bin
    SUFFIX .dol
)
file(COPY ${{PROJECT_SOURCE_DIR}}/meta.xml ${{BINFILES}} DESTINATION ${{PROJECT_SOURCE_DIR}}/bin)
add_custom_target(${{TARGET}}_run COMMAND wiiload ${{PROJECT_SOURCE_DIR}}/bin/boot.dol DEPENDS ${{TARGET}})
"""

    write_file(proj / "CMakeLists.txt", proj_cmake)

    meta = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
\t<name>{name}</name>
\t<coder>{author}</coder>
\t<version>1.0</version>
\t<release_date>{date.today().strftime("%Y%m%d%H%M%S")}</release_date>
\t<short_description>Short description</short_description>
\t<long_description>Long description</long_description>
</app>
"""
    write_file(proj / "meta.xml", meta)

    icon_path = proj / "data" / "icon.png"
    if not icon_path.exists():
        generate_icon(icon_path, name)


def main():
    parser = argparse.ArgumentParser(prog=Path(sys.argv[0]).name)
    sub = parser.add_subparsers(dest="cmd", required=True)
    default_path = get_windows_remote() if os.name in {"nt", "dos"} else Path("/media/usb")

    sub.add_parser("list", help="List all projects on Wii SD/USB")
    sub.add_parser("upload", help="Upload a local project (choose from list)")
    sub.add_parser("upload-all", help="Upload all local projects")
    sub.add_parser("init", help="Create a new project")
    parser.add_argument("-r", "--remote", type=Path, default=default_path,
                        help=f"Path to mounted Wii SD/USB (default: {default_path})")

    args = parser.parse_args()

    if args.cmd == "list":
        cmd_list(args.remote)
    elif args.cmd == "upload":
        cmd_upload(args.remote)
    elif args.cmd == "upload-all":
        cmd_upload_all(args.remote)
    elif args.cmd == "init":
        cmd_init()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting...")
        sys.exit(1)
