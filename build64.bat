@echo off
rem ===========================================================================
rem build64.bat - build GV (Graphic Viewer) as a 64-bit Windows program.
rem
rem Uses the portable llvm-mingw (clang) toolchain, which needs no installer:
rem just a zip download from https://github.com/mstorsjo/llvm-mingw/releases
rem If the toolchain is not already on disk, it is downloaded and extracted
rem automatically.  Requires Windows 10 1803 or later (for curl.exe / tar.exe;
rem PowerShell fallbacks are used when they are missing).
rem The script can be run from a plain command prompt, by double-clicking it
rem in Explorer, or from a Git Bash shell ("cmd //c build64.bat" or
rem "./build64.bat").
rem
rem The official build uses MSVC + nmake (see Makefile / appveyor.yml).  This
rem script reproduces it with clang; the C code is portable to both.
rem
rem The resource compiler in this toolchain is stricter than MSRC, so gv.rc
rem is written in the canonical RC syntax (memory flags dropped, explicit
rem commas between control labels and ids, "EDITTEXT," typo fixed); both
rem MSRC and windres accept it.
rem ===========================================================================
setlocal

rem ---- Configuration --------------------------------------------------------
rem Toolchain release (portable zip, no installer needed):
set "LLVM_MINGW_VERSION=20261020"
set "LLVM_MINGW_ZIP=llvm-mingw-%LLVM_MINGW_VERSION%-ucrt-x86_64.zip"
set "LLVM_MINGW_URL=https://github.com/mstorsjo/llvm-mingw/releases/download/%LLVM_MINGW_VERSION%/%LLVM_MINGW_ZIP%"
rem Where to look for / download the toolchain (override with TOOLCHAIN_DIR):
if "%TOOLCHAIN_DIR%"=="" set "TOOLCHAIN_DIR=C:\tools\mingw64-dl"

cd /d "%~dp0"

rem ---- Locate or download the compiler --------------------------------------
rem Use the absolute paths of the Windows system curl and tar.  A Git Bash
rem environment puts its own MSYS tools first on PATH, and GNU tar cannot
rem handle "C:\..." paths ("tar: Cannot connect to C: resolve failed"), so
rem a plain "tar" lookup is not portable.
set "CURL=%SystemRoot%\System32\curl.exe"
set "TAR=%SystemRoot%\System32\tar.exe"

set "CC_DIR="
for /d %%D in ("%TOOLCHAIN_DIR%\llvm-mingw-*-ucrt-x86_64") do set "CC_DIR=%%~fD"
if defined CC_DIR goto :have_toolchain

echo llvm-mingw toolchain not found in %TOOLCHAIN_DIR%
if not exist "%TOOLCHAIN_DIR%" mkdir "%TOOLCHAIN_DIR%"

set "ZIP_PATH=%TOOLCHAIN_DIR%\%LLVM_MINGW_ZIP%"
if exist "%ZIP_PATH%" goto :extract

echo Downloading %LLVM_MINGW_URL% ...
if exist "%CURL%" (
    "%CURL%" -L -o "%ZIP_PATH%" "%LLVM_MINGW_URL%" || goto :dl_fail
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "$ProgressPreference='SilentlyContinue'; Invoke-WebRequest -Uri '%LLVM_MINGW_URL%' -OutFile '%ZIP_PATH%'" || goto :dl_fail
)

:extract
echo Extracting %ZIP_PATH% ...
if exist "%TAR%" (
    "%TAR%" -xf "%ZIP_PATH%" -C "%TOOLCHAIN_DIR%" || goto :dl_fail
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Expand-Archive -Path '%ZIP_PATH%' -DestinationPath '%TOOLCHAIN_DIR%' -Force" || goto :dl_fail
)

for /d %%D in ("%TOOLCHAIN_DIR%\llvm-mingw-*-ucrt-x86_64") do set "CC_DIR=%%~fD"
if not defined CC_DIR goto :dl_fail

:have_toolchain
if not exist "%CC_DIR%\bin\x86_64-w64-mingw32-clang.exe" (
    echo ERROR: compiler not found in %CC_DIR%\bin
    exit /b 1
)
echo Using toolchain: %CC_DIR%

rem ---- Build -----------------------------------------------------------------
if exist build64 rmdir /s /q build64
if exist build64 goto :fail
mkdir build64

set "CC=%CC_DIR%\bin\x86_64-w64-mingw32-clang.exe"
set "WINDRES=%CC_DIR%\bin\x86_64-w64-mingw32-windres.exe"
set "AR=%CC_DIR%\bin\x86_64-w64-mingw32-ar.exe"

rem All warnings are failures.  The two exceptions are codebase idioms:
rem - pointer-sign: char * and uchar * are used interchangeably throughout
rem - unused-parameter: Windows callback signatures require unused parameters
set "WARN=-Wall -Wextra -Wno-pointer-sign -Wno-unused-parameter -Werror"
rem Optimized for maximum executable speed:
rem  -O3            - aggressive optimization
rem  -march=x86-64-v2 - use SSE4.2/POPCNT (any x86-64 CPU since ~2009)
rem  -flto          - whole-program optimization at link time
set "CFLAGS=-O3 -march=x86-64-v2 -flto -fms-extensions %WARN% -I."
set "LINKFLAGS=-march=x86-64-v2 -flto"

rem Resource file
"%WINDRES%" -I . gv.rc -O coff -o build64\gv.res || goto :fail

for %%F in (gload gsave lzw gv gvlib mem process lib) do (
    "%CC%" -c %CFLAGS% %%F.c -o build64\%%F.o || goto :fail
)

rem gv.exe
"%CC%" build64\gload.o build64\gsave.o build64\lzw.o build64\gv.o build64\gvlib.o build64\mem.o build64\process.o build64\gv.res %LINKFLAGS% -ladvapi32 -lcomdlg32 -lgdi32 -lkernel32 -lshell32 -luser32 -ldwmapi -o build64\gv.exe -mwindows || goto :fail
if not exist build64\gv.exe goto :fail

rem gvlib.dll
"%CC%" build64\lib.o build64\mem.o %LINKFLAGS% -ladvapi32 -lcomdlg32 -lgdi32 -lkernel32 -lshell32 -luser32 -o build64\gvlib.dll -shared || goto :fail

rem Import libraries
"%AR%" rcs build64\gvlib.lib build64\lib.o || goto :fail
"%AR%" rcs build64\mem.lib build64\mem.o || goto :fail

rem Test tools (used by tests\interop.py and tests\asmconv_test.c).
rem They use the gvlib API (LIBRARY-defined code), so link the gvlib.lib
rem archive (built from lib.o) plus mem.o.
"%CC%" -c %CFLAGS% tests\rawdump.c -o build64\rawdump.o || goto :fail
"%CC%" -c %CFLAGS% tests\asmconv_test.c -o build64\asmconv_test.o || goto :fail
"%CC%" build64\rawdump.o build64\gvlib.lib build64\mem.o %LINKFLAGS% -ladvapi32 -lcomdlg32 -lgdi32 -lkernel32 -lshell32 -luser32 -o build64\rawdump.exe || goto :fail
"%CC%" build64\asmconv_test.o build64\gvlib.lib build64\mem.o %LINKFLAGS% -ladvapi32 -lcomdlg32 -lgdi32 -lkernel32 -lshell32 -luser32 -o build64\asmconv_test.exe || goto :fail

echo.
echo Build succeeded:
dir /b build64\gv.exe build64\gvlib.dll build64\gvlib.lib build64\mem.lib build64\rawdump.exe build64\asmconv_test.exe
exit /b 0

:dl_fail
echo ERROR: failed to download or extract the llvm-mingw toolchain.
exit /b 1

:fail
echo ERROR: build failed.
exit /b 1
