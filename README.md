# Graphic Viewer

[![Build status](https://github.com/manthey/gv/actions/workflows/build.yml/badge.svg)](https://github.com/manthey/gv/actions/workflows/build.yml)

Graphic Viewer is a small, fast, free image display program for Windows.

Graphic Viewer supports BMP, CUR, GIF, GROB, ICO, JPG, PCX, PPM, TGA, and TIF format images.  Files do not need the correct extension for GV to recognize and read them.  If you have Python and PIL installed on your system, additional formats (such as PNG) are supported, too.

## Building

The official CI build compiles GV with two compilers: MSVC + nmake as a confirmation build, and llvm-mingw (clang), whose artifacts are the ones published.

On Windows, build GV by running `build64.bat` from the repository root.  It works from a command prompt, from Explorer, or from a Git Bash shell, and it locates or downloads a portable llvm-mingw (clang) toolchain automatically if one is not already installed.  The build produces `build64\gv.exe`, `gvlib.dll`, the import libraries, and the test tools.  A Git Bash shell can also run `build-llvm-mingw64.sh` directly.

## Code style

Formatting is defined by the hooks, not by this document: the style is whatever `.clang-format` says, and the pre-commit hooks in `.pre-commit-config.yaml` enforce and auto-correct it.

    pip install pre-commit
    pre-commit install

clang-format comes from the same llvm-mingw toolchain the build uses (see `hooks/clang_format.py`), so running `build64.bat` once is enough to enable the hooks.  If a commit is aborted because a hook rewrote files, re-add them and commit again.  To fix the whole tree at once: `pre-commit run --all-files`.
