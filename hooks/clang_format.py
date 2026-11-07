#!/usr/bin/env python
"""clang-format wrapper used by the pre-commit hook.

Locates clang-format the same way the build scripts do:

  1. $LLVM_MINGW/bin          (build-llvm-mingw64.sh)
  2. $TOOLCHAIN_DIR/llvm-mingw-*-ucrt-x86_64/bin,
     default C:\tools\mingw64-dl (build64.bat)
  3. clang-format on PATH     (any LLVM or MinGW installation)

The llvm-mingw toolchain is the same one build64.bat downloads, so anyone
who can build GV can run the formatting hook; no separate clang-format
installation is required.  The style itself is pinned by .clang-format at
the repository root, so all writers of the code produce the same result.

Passes all arguments through to clang-format.
"""

import glob
import os
import shutil
import subprocess
import sys


def candidates():
    llvm_mingw = os.environ.get("LLVM_MINGW")
    if llvm_mingw:
        bin_dir = os.path.join(llvm_mingw, "bin")
        yield from (os.path.join(bin_dir, "clang-format.exe"),
                    os.path.join(bin_dir, "clang-format"))
    toolchain_dir = os.environ.get("TOOLCHAIN_DIR", r"C:\tools\mingw64-dl")
    for bin_dir in sorted(glob.glob(
            os.path.join(toolchain_dir, "llvm-mingw-*-ucrt-x86_64", "bin"))):
        yield from (os.path.join(bin_dir, "clang-format.exe"),
                    os.path.join(bin_dir, "clang-format"))
    yield shutil.which("clang-format")


def find_clang_format():
    for candidate in candidates():
        if candidate and os.path.isfile(candidate):
            return candidate
    sys.exit(
        "clang-format not found.\n"
        "It comes with the llvm-mingw toolchain used to build GV.  Run\n"
        "  build64.bat            (Windows command prompt)\n"
        "  ./build-llvm-mingw64.sh (Git Bash)\n"
        "which downloads it if needed, or install LLVM/clang-format and put\n"
        "it on PATH, or set LLVM_MINGW to the toolchain directory.")


def main():
    command = [find_clang_format(), *sys.argv[1:]]
    try:
        result = subprocess.run(command)
    except OSError as error:
        sys.exit("failed to run %s: %s" % (command[0], error))
    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
