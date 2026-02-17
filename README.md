# WiiSandbox

> Sandbox for custom Wii homebrew development.

## Development Guide

### Setup devkitPro and devkitPPC

Follow the guide: https://devkitpro.org/wiki/Getting_Started

### Clone this repository

```bash
$ git clone https://github.com/Sid110307/WiiSandbox.git 
$ cd WiiSandbox
```

### Install Python dependencies (optional)

Only required to generate a default `icon.png` for new projects.
You can also create your own `icon.png` using any image editor (must be 128x48 pixels).

```bash
$ pip install Pillow
```

### Run the Python script

```bash
$ python3 run.py --help
usage: run.py [-h] [-r REMOTE] {list,upload,upload-all,init} ...

positional arguments:
  {list,upload,upload-all,init}
    list        List all projects on Wii SD/USB
    upload      Upload a local project (choose from list)
    upload-all  Upload all local projects
    init        Create a new project

optional arguments:
  -h, --help            show this help message and exit
  -r REMOTE, --remote REMOTE
                        Path to mounted Wii SD/USB (default: /media/usb)
```

## Python Script Commands

### Create a New Project

```bash
$ python3 run.py init
Project name: ExampleProject1
Author: John Doe
```

### List Projects on Wii SD/USB

```bash
$ python3 run.py list
Projects in /media/usb:
  1) ExampleProject1
  2) ExampleProject2
```

### Upload a Project

```bash
$ python3 run.py upload
  1) ExampleProject3 (boot.dol)
Choose a project to upload: 1
Uploaded project to /media/usb/apps/ExampleProject1.
```

### Upload All Projects

```bash
$ python3 run.py upload-all
Uploaded project to /media/usb/apps/ExampleProject1.
Uploaded project to /media/usb/apps/ExampleProject2.
```

> ### Remote Path
>
> By default, the script looks for the Wii SD/USB at `/media/usb` or the first found mounted drive. You can specify a
> custom path using the `-r` or `--remote` option.
>
> ```bash
> $ python3 run.py list -r /path/to/mounted/drive
> $ python3 run.py list -r E:
> ```

> ### Project Structure
>
> ```
> ProjectName/
> ├── bin/            # Compiled binaries (boot.dol)
> ├── src/            # Source code files (C/C++)
> ├── data/           # Assets (images, sounds, etc.)
>     ├── icon.png    # 128x48 icon
> ├── CMakeLists.txt  # Build configuration file
> ├── meta.xml        # Metadata file
> ```
>
> ```
> apps/<ProjectName>/
> ├── boot.dol  # Compiled homebrew executable
> ├── icon.png  # 128x48 icon
> ├── meta.xml  # Metadata file
> ```

## License

[MIT](https://opensource.org/licenses/MIT)
