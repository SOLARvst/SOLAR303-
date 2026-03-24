@echo off
setlocal enabledelayedexpansion

echo ============================================
echo   TB-303 Clone - Windows Build Script
echo ============================================
echo.

where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake not found.
    pause
    exit /b 1
)

where git >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Git not found.
    pause
    exit /b 1
)

echo [OK] CMake and Git found.
echo.

cd /d "%~dp0.."

echo [1/3] Configuring project (this downloads JUCE on first run)...

if exist "build_windows" rmdir /s /q build_windows
mkdir build_windows

cmake -S . -B build_windows -A x64 -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] CMake configuration failed.
    pause
    exit /b 1
)

echo.
echo [2/3] Building (Release)...
cmake --build build_windows --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

echo.
echo [3/3] Build complete!
echo.
echo VST3 plugin is in:
echo build_windows\TB303Clone_artefacts\Release\VST3\
echo.
pause
