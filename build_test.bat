@echo off
REM Build script for ImFileBrowser scaling test
REM This builds the test application separately from the main library

setlocal

REM Configuration
set BUILD_DIR=build_test
set CONFIG=Debug

REM Find vcpkg toolchain
if defined VCPKG_ROOT (
    set TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
) else if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
) else if exist "%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set TOOLCHAIN_FILE=%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake
) else (
    echo.
    echo ERROR: Cannot find vcpkg toolchain file.
    echo Please set VCPKG_ROOT environment variable.
    echo.
    exit /b 1
)

echo Using vcpkg toolchain: %TOOLCHAIN_FILE%

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Run CMake
echo.
echo === Configuring with CMake ===
cd "%BUILD_DIR%"

cmake ..\test -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%"

if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    echo.
    echo Make sure you have:
    echo   1. GLFW installed, vcpkg install glfw3:x64-windows
    echo   2. ImGui available, vcpkg install imgui[glfw-binding,opengl3-binding]:x64-windows
    echo.
    cd ..
    exit /b 1
)

REM Build
echo.
echo === Building ===
cmake --build . --config %CONFIG%
if errorlevel 1 (
    echo.
    echo Build failed!
    cd ..
    exit /b 1
)

echo is it done

cd ..

echo.
echo === Build successful! ===
echo.
echo Run the test with:
echo   %BUILD_DIR%\bin\%CONFIG%\ImFileBrowserTest.exe
echo.
echo Controls:
echo   CTRL+PLUS  - Increase UI scale
echo   CTRL+MINUS - Decrease UI scale
echo   CTRL+0     - Reset scale to 1.0
echo.

REM Optionally run the test
if "%1"=="run" (
    echo Running test...
    "%BUILD_DIR%\bin\%CONFIG%\ImFileBrowserTest.exe"
)
