# Graphic Viewer

[![Build status](https://github.com/manthey/gv/actions/workflows/build.yml/badge.svg)](https://github.com/manthey/gv/actions/workflows/build.yml)

Graphic Viewer is a small, fast, free image display program for Windows.

Graphic Viewer supports BMP, CUR, GIF, GROB, ICO, JPG, PCX, PPM, TGA, and TIF format images.  Files do not need the correct extension for GV to recognize and read them.  If you have Python and PIL installed on your system, additional formats (such as PNG) are supported, too.

## Building

The official CI build compiles GV with two compilers: MSVC + nmake as a confirmation build, and llvm-mingw (clang), whose artifacts are the ones published.

On Windows, build GV by running `build64.bat` from the repository root.  It works from a command prompt, from Explorer, or from a Git Bash shell, and it locates or downloads a portable llvm-mingw (clang) toolchain automatically if one is not already installed.  The build produces `build64\gv.exe`, `gvlib.dll`, the import libraries, and the test tools.  A Git Bash shell can also run `build-llvm-mingw64.sh` directly.

## Code style

The C code is formatted with clang-format; the style is defined in `.clang-format`.  It keeps the historical look of the codebase (2-space indent, braces on the same line as the control statement) while normalizing spacing, line lengths (100 columns), and tabs mechanically.  The historical banner comment blocks above each function (the `* Enter: ...  5/4/96-DWM *` style annotations) are preserved.

Formatting is enforced automatically with pre-commit:

    pip install pre-commit
    pre-commit install

The hooks also fix trivial whitespace violations (trailing whitespace, missing end-of-file newlines, mixed line endings).  The clang-format hook runs in fixing mode: if a commit contains format violations, the files are rewritten and the commit is aborted, so re-add the corrected files and commit again (`git add -u && git commit`).

clang-format comes from the same llvm-mingw toolchain the build uses; it is found via `LLVM_MINGW`, `TOOLCHAIN_DIR`, or `PATH` (see `hooks/clang_format.py`).  Running `build64.bat` once is therefore enough to enable the hooks.  To fix the whole tree at once, run `pre-commit run --all-files`.
