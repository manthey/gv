# Graphic Viewer

[![Build status](https://github.com/manthey/gv/actions/workflows/build.yml/badge.svg)](https://github.com/manthey/gv/actions/workflows/build.yml)

Graphic Viewer is a small, fast, free image display program for Windows.

Graphic Viewer supports BMP, CUR, GIF, GROB, ICO, JPG, PCX, PPM, TGA, and TIF format images.  Files do not need the correct extension for GV to recognize and read them.  If you have Python and PIL installed on your system, additional formats (such as PNG) are supported, too.

## Building

On Windows, build GV by running `build64.bat` from the repository root.  It works from a command prompt, from Explorer, or from a Git Bash shell, and it locates or downloads a portable llvm-mingw (clang) toolchain automatically if one is not already installed.  The build produces `build64\gv.exe`, `gvlib.dll`, the import libraries, and the test tools.

The official CI build uses MSVC + nmake (see `Makefile`).  A Git Bash shell can also run `build-llvm-mingw64.sh` directly.

