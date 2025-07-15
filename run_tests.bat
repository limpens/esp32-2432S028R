@echo off
REM ESP32-2432S028R Test Runner Script for Windows
REM Usage: run_tests.bat [host|unity|build|all|clean|help]

setlocal enabledelayedexpansion

set PROJECT_ROOT=%~dp0
cd /d "%PROJECT_ROOT%"

if "%1"=="" set COMMAND=all
if not "%1"=="" set COMMAND=%1

echo ========================================
echo  ESP32-2432S028R Test Suite
echo ========================================

if "%COMMAND%"=="host" goto run_host_tests
if "%COMMAND%"=="unity" goto run_unity_tests  
if "%COMMAND%"=="build" goto run_main_build
if "%COMMAND%"=="all" goto run_all_tests
if "%COMMAND%"=="clean" goto cleanup
if "%COMMAND%"=="help" goto show_help

echo Unknown command: %COMMAND%
goto show_help

:run_host_tests
echo.
echo [HOST TESTS] Building and running host tests...
cd test\host_test

if not exist build mkdir build
cd build

cmake .. || goto host_test_failed
cmake --build . || goto host_test_failed
host_tests.exe || goto host_test_failed

echo [SUCCESS] Host tests passed!
cd /d "%PROJECT_ROOT%"
goto :eof

:host_test_failed
echo [ERROR] Host tests failed!
cd /d "%PROJECT_ROOT%"
exit /b 1

:run_unity_tests
echo.
echo [UNITY TESTS] Building Unity tests for ESP32...
cd test\unity

REM Check if ESP-IDF is available
idf.py --version >nul 2>&1 || goto no_esp_idf

idf.py set-target esp32 || goto unity_test_failed
idf.py build || goto unity_test_failed

echo [SUCCESS] Unity tests built successfully!
echo To run on device: idf.py -p COM3 flash monitor
cd /d "%PROJECT_ROOT%"
goto :eof

:unity_test_failed
echo [ERROR] Unity tests build failed!
cd /d "%PROJECT_ROOT%"
exit /b 1

:run_main_build
echo.
echo [MAIN BUILD] Building main project...

REM Check if ESP-IDF is available
idf.py --version >nul 2>&1 || goto no_esp_idf

idf.py set-target esp32 || goto main_build_failed
idf.py build || goto main_build_failed

echo [SUCCESS] Main project built successfully!
goto :eof

:main_build_failed
echo [ERROR] Main project build failed!
exit /b 1

:run_all_tests
echo.
echo [ALL TESTS] Running complete test suite...

set FAILED=0

call :run_host_tests
if errorlevel 1 set FAILED=1

call :run_main_build  
if errorlevel 1 set FAILED=1

call :run_unity_tests
if errorlevel 1 set FAILED=1

if !FAILED!==0 (
    echo.
    echo [SUCCESS] All tests and builds completed successfully!
) else (
    echo.
    echo [ERROR] Some tests or builds failed!
    exit /b 1
)
goto :eof

:cleanup
echo.
echo [CLEANUP] Cleaning build artifacts...

if exist build (
    rmdir /s /q build
    echo Cleaned main build directory
)

if exist test\unity\build (
    rmdir /s /q test\unity\build
    echo Cleaned Unity test build directory
)

if exist test\host_test\build (
    rmdir /s /q test\host_test\build
    echo Cleaned host test build directory
)

echo [SUCCESS] Cleanup completed!
goto :eof

:no_esp_idf
echo [ERROR] ESP-IDF not found. Please install and activate ESP-IDF first.
echo See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/
exit /b 1

:show_help
echo.
echo ESP32-2432S028R Test Runner
echo.
echo Usage: %0 [COMMAND]
echo.
echo Commands:
echo   host     Run host-based tests (fast, no hardware needed)
echo   unity    Build Unity tests for ESP32 device
echo   build    Build main project
echo   all      Run all tests and builds
echo   clean    Clean all build artifacts  
echo   help     Show this help message
echo.
echo Examples:
echo   %0 host          # Quick validation
echo   %0 unity         # Prepare device tests
echo   %0 all           # Full CI-like test run
goto :eof
