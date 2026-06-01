@echo off
REM Configure and build HEIF with Visual Studio (64-bit Release).
REM Works with VS 2017, 2019, and 2022 -- CMake picks the latest installed.
REM To target a specific version, add e.g. -G "Visual Studio 17 2022"

cmake ..\srcs -A x64
cmake --build . --config Release
