@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "BUILD_ARGS=-Board ""nrf52840dk/nrf52840"""
set "FLASH_ARGS=-Runner jlink"

:parse_args
if "%~1"=="" goto run_steps

if /I "%~1"=="-Pristine" (
    set "BUILD_ARGS=%BUILD_ARGS% -Pristine"
    shift
    goto parse_args
)

if /I "%~1"=="-Recover" (
    set "FLASH_ARGS=%FLASH_ARGS% -Recover"
    shift
    goto parse_args
)

if /I "%~1"=="-DevId" (
    if "%~2"=="" (
        echo [ERROR] -DevId requires a value.
        pause
        exit /b 1
    )
    set "FLASH_ARGS=%FLASH_ARGS% -DevId %~2"
    shift
    shift
    goto parse_args
)

echo [ERROR] Unsupported argument: %~1
echo [INFO] Supported arguments: -Pristine  -Recover  -DevId SERIAL
pause
exit /b 1

:run_steps
echo [INFO] Build firmware...
powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%build.ps1" %BUILD_ARGS%
set "EXIT_CODE=%ERRORLEVEL%"
if not "%EXIT_CODE%"=="0" (
    echo [ERROR] Build failed, exit code: %EXIT_CODE%
    pause
    exit /b %EXIT_CODE%
)

echo [INFO] Download firmware with J-Link...
powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%flash.ps1" %FLASH_ARGS%
set "EXIT_CODE=%ERRORLEVEL%"
if not "%EXIT_CODE%"=="0" (
    echo [ERROR] Download failed, exit code: %EXIT_CODE%
    pause
    exit /b %EXIT_CODE%
)

echo [INFO] Build and download completed.
pause
exit /b 0
