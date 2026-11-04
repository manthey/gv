#!/bin/sh
# Build GV (Graphic Viewer) with LLVM-MinGW (clang), 64-bit, from a Git Bash
# shell.
#
# The official build uses MSVC + nmake (see Makefile and appveyor.yml).  This
# script reproduces it with a portable LLVM-MinGW x86_64 toolchain:
#
# The resource compiler in this toolchain is stricter than MSRC, so gv.rc is
# written in the canonical RC syntax (memory flags dropped, explicit commas
# between control labels and ids, "EDITTEXT," typo fixed); both MSRC and
# windres accept it.

set -e
cd "$(dirname "$0")"

LLVM_MINGW=${LLVM_MINGW:-/c/tools/mingw64-dl/llvm-mingw-20261020-ucrt-x86_64}
PATH="$LLVM_MINGW/bin:$PATH"
CC=x86_64-w64-mingw32-clang
WINDRES=x86_64-w64-mingw32-windres
AR=x86_64-w64-mingw32-ar

# All warnings are failures.  The two exceptions are codebase idioms:
# - pointer-sign: char * and uchar * are used interchangeably throughout
# - unused-parameter: Windows callback signatures require unused parameters
WARN="-Wall -Wextra -Wno-pointer-sign -Wno-unused-parameter -Werror"
CFLAGS="-O2 -fms-extensions $WARN -I."
LIBS="-ladvapi32 -lcomdlg32 -lgdi32 -lkernel32 -lshell32 -luser32 -ldwmapi"

rm -rf build64
mkdir -p build64

# Resource file
$WINDRES -I . gv.rc -O coff -o build64/gv.res

# Objects
for f in gload gsave lzw gv gvlib mem process lib; do
    $CC -c $CFLAGS $f.c -o build64/$f.o
done

# gv.exe
$CC build64/gload.o build64/gsave.o build64/lzw.o build64/gv.o \
    build64/gvlib.o build64/mem.o build64/process.o build64/gv.res \
    $LIBS -o build64/gv.exe -mwindows

# gvlib.dll
$CC build64/lib.o build64/mem.o $LIBS -o build64/gvlib.dll -shared

# Import libraries
$AR rcs build64/gvlib.lib build64/lib.o
$AR rcs build64/mem.lib build64/mem.o

ls -la build64/gv.exe build64/gvlib.dll build64/gvlib.lib build64/mem.lib
