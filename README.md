# Yukon Solitaire — Project README

> Implemented in portable ISO C11 with a single‑header engine, a text UI (TUI)
> and an SDL2 GUI.  Works on Linux, macOS, and Windows (MinGW or MSVC).

---
## Table of Contents
1. [Repository layout](#repository-layout)
2. [Prerequisites](#prerequisites)
3. [Quick start](#quick-start)
4. [Build & run](#build--run)
   * [Using the Makefile](#using-the-makefile)
   * [Manual GCC/Clang commands](#manual-commands)
   * [Building on Windows](#windows)
5. [Command cheat‑sheet](#command-cheat-sheet)
6. [Automated/unit tests](#automated-unit-tests)
7. [Cleaning the workspace](#cleaning)

---
## Repository layout
| file / dir          | role |
|---------------------|------|
| `yukon_engine.h`    | **single‑header core** — game state & rule‑checking only |
| `tui.c`             | Terminal front‑end (uses only stdio) |
| `gui_sdl.c`         | SDL2 GUI front‑end (raster cards, entry line) |
| `tests.c`           | Stand‑alone regression driver (see §6) |
| `Makefile`          | Convenience targets (`make`, `make tui`, …) |
| `README.md`         | what you are reading |

---
## Prerequisites
| component | Linux / macOS | Windows (MSYS2/MinGW) | purpose |
|-----------|---------------|-----------------------|---------|
| **C compiler** | `gcc`≥4.9 or `clang`≥3.3 | `pacman ‑S mingw‑w64‑x86_64‑gcc` | builds everything |
| **SDL 2** | `sudo apt install libsdl2‑dev`<br>`brew install sdl2` | `pacman ‑S mingw‑w64‑x86_64‑SDL2` | GUI window / input |
| **SDL2_ttf** | `sudo apt install libsdl2‑ttf‑dev`<br>`brew install sdl2_ttf` | `pacman ‑S mingw‑w64‑x86_64‑SDL2_ttf` | font rendering |
| **make** (optional) | usually pre‑installed | `pacman ‑S make` | runs the Makefile |

> *No other external libraries are required.*

---
## Quick start
```bash
# clone / unpack project, then
make             # builds tui, yukon_gui, tests
./bin/tui            # run text UI
./bin/yukon_gui      # run SDL2 GUI
make test        # run unit driver (exit code 0 ⇒ success)
```
If you prefer manual compilation, see next section.

---
## Build & run
### Using the Makefile
| target        | action |
|---------------|--------|
| `make` (default) | builds **tui**, **yukon_gui**, and **tests** |
| `make tui`    | builds terminal version only |
| `make gui`    | builds SDL2 GUI only |
| `make test`   | compiles & executes `tests.c` |
| `make clean`  | removes all build artefacts |

### Manual commands
```bash
# TUI — no external libs
gcc -std=c11 -Wall -Wextra tui.c -o tui

# GUI — needs SDL2 & SDL2_ttf
gcc -std=c11 -Wall -Wextra gui_sdl.c -o yukon_gui \
    $(sdl2-config --cflags) -lSDL2_ttf $(sdl2-config --libs)

# Run
./tui         # or ./yukon_gui
```
If `sdl2-config` is not on your PATH replace the parenthesised part with
explicit `-I` (include path) and `-L`/`-l` (library) flags.

### <a name="windows"></a>Building on Windows (MinGW + MSYS2)
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 \
          mingw-w64-x86_64-SDL2_ttf make
mingw32-make               # run Makefile
```
For MSVC: open the “x64 Native Tools” shell and run
```cmd
cl /std:c11 /W4 /D_CRT_SECURE_NO_WARNINGS tui.c /Fe:tui.exe
cl /std:c11 /W4 gui_sdl.c SDL2.lib SDL2_ttf.lib /Fe:yukon_gui.exe
```
(Ensure SDL2 include/lib directories are in `%INCLUDE%` and `%LIB%`.)

---
## <a name="command-cheat-sheet"></a>Command cheat‑sheet
| phase   | commands |
|---------|----------|
| STARTUP | `LD [file]`, `SW`, `SI [n]`, `SR`, `SD [file]`, `P`, `QQ` |
| PLAY    | `Q` (return to STARTUP), `QQ` (exit), move syntax `src->dst` |
|         | *Examples:* `C3:7H->C5`, `C4->F2`, `F1->C6` |

In the **GUI** you **must type commands in UPPER‑CASE** (`LD`, `P`, `C1`, …).
Characters appear in the white entry bar; press **Enter** to submit.

---
## Automated / unit tests
Running `make test` (or `./tests`) performs:
1. load ordered deck
2. perfect riffle shuffle
3. deal tableau and verify column heights
4. quit to STARTUP and ensure snapshot restoration

---
## Cleaning
```bash
make clean    # removes tui, yukon_gui, tests, *.o, core dumps
```