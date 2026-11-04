#!/bin/sh
# Build GV (Graphic Viewer) with LLVM-MinGW (clang), 32-bit, from a Git Bash shell.
#
# The official build uses MSVC + nmake (see Makefile and appveyor.yml).  This
# script reproduces it with a portable LLVM-MinGW i686 toolchain instead:
#   - clang supports the MSVC-style "__asm { ... }" blocks via -fasm-blocks
#   - the build targets i686 (32-bit) because the inline asm is 32-bit x86
#
# Toolchain used (no installer needed, just unzip):
#   https://github.com/mstorsjo/llvm-mingw/releases
#   llvm-mingw-20261020-ucrt-i686.zip  (any recent i686 ucrt build works)
# Download and extract, then set LLVM_MINGW below or on the command line:
#   LLVM_MINGW=/c/tools/mingw64-dl/llvm-mingw-20261020-ucrt-i686 ./build-llvm-mingw.sh
#
# The resource compiler in this toolchain is stricter than MSRC, so gv.rc is
# copied to build/gv.rc with three fixes applied by sed (MSRC tolerated all of
# these; GNU/LLVM windres do not):
#   1. VERSIONINFO/DIALOG memory flags (MOVEABLE IMPURE LOADONCALL DISCARDABLE)
#      are dropped
#   2. missing commas between quoted control labels and control ids are added
#   3. a typo "EDITTEXT," is corrected to "EDITTEXT"

set -e
cd "$(dirname "$0")"

LLVM_MINGW=${LLVM_MINGW:-/c/tools/mingw64-dl/llvm-mingw-20261020-ucrt-i686}
PATH="$LLVM_MINGW/bin:$PATH"
CC=i686-w64-mingw32-clang
WINDRES=i686-w64-mingw32-windres
AR=i686-w64-mingw32-ar

CFLAGS="-O2 -Wno-error=incompatible-pointer-types -fms-extensions -fasm-blocks -I."
LIBS="-ladvapi32 -lcomdlg32 -lgdi32 -lkernel32 -lshell32 -luser32 -ldwmapi"

rm -rf build
mkdir -p build

# Resource file (patched copy for windres syntax)
sed -E 's/^(VersionGV VERSIONINFO).*/\1/;
        s/ DIALOG (LOADONCALL |MOVEABLE |IMPURE |PURE |DISCARDABLE )+/ DIALOG /g;
        s/^(.* DIALOG) +/\1 /;
        s/(ICON [A-Za-z0-9_]+, IDC_STATIC) ([0-9])/\1, \2/;
        s/("[^"]*") ([A-Za-z][A-Za-z0-9_]*)/\1, \2/g;
        s/EDITTEXT, /EDITTEXT /' gv.rc > build/gv.rc
$WINDRES -I . build/gv.rc -O coff -o build/gv.res

# Objects
for f in gload gsave lzw gv gvlib mem process lib; do
    $CC -c $CFLAGS $f.c -o build/$f.o
done

# gv.exe
$CC build/gload.o build/gsave.o build/lzw.o build/gv.o build/gvlib.o build/mem.o \
    build/process.o build/gv.res $LIBS -o build/gv.exe -mwindows

# gvlib.dll
$CC build/lib.o build/mem.o $LIBS -o build/gvlib.dll -shared

# Import libraries
$AR rcs build/gvlib.lib build/lib.o
$AR rcs build/mem.lib build/mem.o

ls -la build/gv.exe build/gvlib.dll build/gvlib.lib build/mem.lib
